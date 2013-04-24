#include <core/zeitkit.h>

#include <yaml-cpp/yaml.h>
#include <happyhttp.h>
#include <json.h>

#ifdef __WIN32__
	#include <winsock2.h>
#endif

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

const char* Zeitkit::fileZeitkit = ".zeitkit";
const char* Zeitkit::pathWorklogs = "worklogs";
const char* Zeitkit::pathClients = "clients";

const char* Zeitkit::remoteAddr = "zeitkit.com";
const unsigned int Zeitkit::remotePort = 80;
const char* Zeitkit::queryLogin = "/sessions";

const char* Zeitkit::globalHeaders[] =
{
	"Content-type", "application/json",
	"Accept", "application/json",
	nullptr
};

Zeitkit::Zeitkit(const char* baseDirectory) : baseDirectory(baseDirectory), initialized(false)
{
#ifdef __WIN32__
	WSADATA data;
	WSAStartup(0x202, &data);
#endif

	YAML::Node init;

	try
	{
		init = YAML::LoadFile(fileZeitkit);
	}
	catch (const YAML::BadFile& bad_file)
	{
		return;
	}

	initialized = true;

	if (init["auth_token"])
		auth_token = init["auth_token"].as<string>();
}

Zeitkit::~Zeitkit()
{
#ifdef __WIN32__
	WSACleanup();
#endif
}

void Zeitkit::write()
{
	YAML::Node init;

	try
	{
		init = YAML::LoadFile(fileZeitkit);
	}
	catch (const YAML::BadFile& bad_file) {}

	init["auth_token"] = auth_token;

	ofstream fout(fileZeitkit);
	fout << init;
}

void Zeitkit::authenticate(const string& input_mail, const string& input_pwd)
{
	try
	{
		happyhttp::Connection conn(remoteAddr, remotePort);

		string query = "{\"email\": \"" + input_mail + "\", \"password\": \"" + input_pwd + "\"}";
		string result;

		static int code;

		conn.setcallbacks(
		[](const happyhttp::Response* resp, void* userdata)
		{
			code = resp->getstatus();
		},
		[](const happyhttp::Response* resp, void* userdata, const unsigned char* data, int n)
		{
			string& result = *reinterpret_cast<string*>(userdata);
			result.append(reinterpret_cast<const char*>(data), n);
		},
		[](const happyhttp::Response* resp, void* userdata)
		{
		}, reinterpret_cast<void*>(&result));

		conn.request("POST", queryLogin, globalHeaders, reinterpret_cast<const unsigned char*>(query.c_str()), query.size());

		while (conn.outstanding())
			conn.pump();

		switch (code)
		{
			case 401:
				cout << "E-Mail / password is invalid. Do you want to create an account? Use the --register option." << endl;
				break;

			case 200:
			{
				char* errorPos = 0;
				char* errorDesc = 0;
				int errorLine = 0;
				block_allocator allocator(1 << 10);

				char* buffer = strdup(result.c_str());

				json_value* root = json_parse(buffer, &errorPos, &errorDesc, &errorLine, &allocator);

				if (root)
				{
					auth_token = root->first_child->string_value;
					write();
				}
				else
					cout << "Malformed server response: " << errorDesc << endl;

				free(buffer);
				break;
			}

			default:
				cout << "Unhandled HTTP status code: " << code << endl;
				break;
		}
	}
	catch (const happyhttp::Wobbly& wobbly)
	{
		cout << "An error occured: " << wobbly.what() << endl;
	}
}

void Zeitkit::init(const char* mail, const char* password, bool register_account, bool force)
{
	if (initialized && !force)
	{
		cout << "Zeitkit has already been initialized in this directory. Use --force to perform the action anyway." << endl;
		return;
	}

	string input_mail;
	string input_pwd;

	if (!mail)
	{
		cout << "Your e-mail address: ";
		cin >> input_mail;

		cout << "Your password: ";
		cin >> input_pwd;
	}
	else
	{
		input_mail = mail;

		if (!password)
		{
			cout << "You entered an e-mail address. Please also give the password with --password yourpassword." << endl;
			return;
		}

		input_pwd = password;
	}

	if (register_account)
	{

	}
	else
		authenticate(input_mail, input_pwd);
}
