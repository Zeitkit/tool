#include <core/zeitkit.h>
#include <core/worklog.h>
#include <utils/input.h>
#include <utils/checksum.h>

#include <yaml-cpp/yaml.h>
#include <happyhttp.h>
#include <json.h>

#ifdef __WIN32__
	#include <winsock2.h>
	#include <direct.h>
#else
	#include <sys/stat.h>
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

using namespace std;

const char* Zeitkit::fileZeitkit = ".zeitkit";
const char* Zeitkit::fileCommits = ".commits";
const char* Zeitkit::fileStatus = ".status";
const char* Zeitkit::pathWorklogs = "worklogs";
const char* Zeitkit::pathClients = "clients";

const char* Zeitkit::remoteAddr = "foxtacles.com";
const unsigned int Zeitkit::remotePort = 3000;
const char* Zeitkit::queryLogin = "/sessions";
const char* Zeitkit::queryRegister = "/users";
const char* Zeitkit::queryWorklogs = "/worklogs";

const char* Zeitkit::globalHeaders[] =
{
	"Content-type", "application/json",
	"Accept", "application/json",
	nullptr
};

Zeitkit::Zeitkit(const char* baseDirectory) : baseDirectory(baseDirectory), initialized(false), last_update(0)
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

	if (init["last_update"])
		last_update = init["last_update"].as<unsigned int>();
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
	init["last_update"] = last_update;

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
		[](const happyhttp::Response* resp, void*)
		{
			code = resp->getstatus();
		},
		[](const happyhttp::Response*, void* userdata, const unsigned char* data, int n)
		{
			string& result = *reinterpret_cast<string*>(userdata);
			result.append(reinterpret_cast<const char*>(data), n);
		},
		[](const happyhttp::Response*, void*)
		{
		}, reinterpret_cast<void*>(&result));

		conn.request("POST", queryLogin, globalHeaders, reinterpret_cast<const unsigned char*>(query.c_str()), query.size());

		while (conn.outstanding())
			conn.pump();

		switch (code)
		{
			case happyhttp::UNAUTHORIZED:
				cout << "E-Mail / password is invalid. Do you want to create an account? Run zeitkit init --register." << endl;
				break;

			case happyhttp::OK:
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

void Zeitkit::register_account(const std::string& input_mail, const std::string& input_pwd)
{
	try
	{
		happyhttp::Connection conn(remoteAddr, remotePort);

		string query = "{\"email\": \"" + input_mail + "\", \"password\": \"" + input_pwd + "\"}";
		string result;

		static int code;

		conn.setcallbacks(
		[](const happyhttp::Response* resp, void*)
		{
			code = resp->getstatus();
		},
		[](const happyhttp::Response*, void* userdata, const unsigned char* data, int n)
		{
			string& result = *reinterpret_cast<string*>(userdata);
			result.append(reinterpret_cast<const char*>(data), n);
		},
		[](const happyhttp::Response*, void*)
		{
		}, reinterpret_cast<void*>(&result));

		conn.request("POST", queryRegister, globalHeaders, reinterpret_cast<const unsigned char*>(query.c_str()), query.size());

		while (conn.outstanding())
			conn.pump();

		char* errorPos = 0;
		char* errorDesc = 0;
		int errorLine = 0;
		block_allocator allocator(1 << 10);

		char* buffer = strdup(result.c_str());
		json_value* root = json_parse(buffer, &errorPos, &errorDesc, &errorLine, &allocator);

		if (root)
		{
			switch (code)
			{
				case happyhttp::BAD_REQUEST:
				{
					cout << "Errors occured on registration, sorry!" << endl;

					if (root->first_child->type == JSON_ARRAY)
						for (json_value* it = root->first_child->first_child; it; it = it->next_sibling)
							cout << it->string_value << endl;
					break;
				}

				case happyhttp::CREATED:
				{
					auth_token = root->first_child->string_value;
					write();
					break;
				}

				default:
					cout << "Unhandled HTTP status code: " << code << endl;
					break;
			}
		}
		else
			cout << "Malformed server response: " << errorDesc << endl;

		free(buffer);
	}
	catch (const happyhttp::Wobbly& wobbly)
	{
		cout << "An error occured: " << wobbly.what() << endl;
	}
}

string Zeitkit::validate_unchanged()
{
	YAML::Node node;

	string result;

	try
	{
		node = YAML::LoadFile(string(pathWorklogs) + "/" + fileCommits);

		if (node.size())
		{
			char buf[16];
			snprintf(buf, sizeof(buf), "%d", node.size());
			result += string("You have ") + buf + " pending commits\n";
		}
	}
	catch (const YAML::BadFile& bad_file) {}

	try
	{
		node = YAML::LoadFile(string(pathWorklogs) + "/" + fileStatus);

		if (node.IsMap())
		{
			map<string, unsigned int> files = node.as<map<string, unsigned int>>();

			for (const auto& file : files)
			{
				unsigned int crc = 0;
				string file_ = string(pathWorklogs) + "/" + file.first + ".worklog";
				Utils::crc32file(file_.c_str(), &crc);

				if (crc != file.second)
					result += "File has been removed or changed: " + file_ + "\n";
			}
		}
	}
	catch (const YAML::BadFile& bad_file) {}

	return result;
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

		Utils::SetStdinEcho(false);
		cin >> input_pwd;
		cout << endl;
		Utils::SetStdinEcho(true);
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
		this->register_account(input_mail, input_pwd);
	else
		authenticate(input_mail, input_pwd);
}

void Zeitkit::pull()
{
	if (!initialized || auth_token.empty())
	{
		cout << "Please initialize this directory first: zeitkit init." << endl;
		return;
	}

	string validate = validate_unchanged();

	if (validate.empty())
	{
		mkdir(pathWorklogs);
		mkdir(pathClients);

		try
		{
			happyhttp::Connection conn(remoteAddr, remotePort);

			char buf[16];
			snprintf(buf, sizeof(buf), "%d", last_update);

			string query = string("{\"updated_since\": ") + buf + ", \"access_token\": \"" + auth_token + "\"}";
			string result;

			static int code;

			conn.setcallbacks(
			[](const happyhttp::Response* resp, void*)
			{
				code = resp->getstatus();
			},
			[](const happyhttp::Response*, void* userdata, const unsigned char* data, int n)
			{
				string& result = *reinterpret_cast<string*>(userdata);
				result.append(reinterpret_cast<const char*>(data), n);
			},
			[](const happyhttp::Response*, void*)
			{
			}, reinterpret_cast<void*>(&result));

			conn.request("GET", queryWorklogs, globalHeaders, reinterpret_cast<const unsigned char*>(query.c_str()), query.size());

			while (conn.outstanding())
				conn.pump();

			switch (code)
			{
				case happyhttp::UNAUTHORIZED:
					cout << "Your authentication has expired. Please use zeitkit auth to re-authenticate with the server." << endl;
					break;

				case happyhttp::OK:
				{
					char* errorPos = 0;
					char* errorDesc = 0;
					int errorLine = 0;
					block_allocator allocator(1 << 10);

					char* buffer = strdup(result.c_str());

					json_value* root = json_parse(buffer, &errorPos, &errorDesc, &errorLine, &allocator);

					if (root && root->type == JSON_ARRAY)
					{
						unsigned int count_updated = 0;
						unsigned int count_deleted = 0;

						YAML::Node status;
						string status_file = string(pathWorklogs) + "/" + fileStatus;

						try
						{
							status = YAML::LoadFile(status_file);
						}
						catch (const YAML::BadFile& bad_file) {}

						for (json_value* it = root->first_child; it; it = it->next_sibling)
						{
							Worklog worklog(last_update, it);
							string file_name = string(pathWorklogs) + "/" + worklog.GetFileName();
							string key = worklog.GetIdString();

							if (worklog.IsUpdated())
							{
								YAML::Node node;
								node = worklog;

								ofstream fout(file_name);
								fout << node;
								fout.flush();

								++count_updated;

								unsigned int crc = 0;
								Utils::crc32file(file_name.c_str(), &crc);
								status[key] = crc;
							}
							else if (worklog.IsDeleted())
							{
								remove(file_name.c_str());
								++count_deleted;

								if (status[key])
									status.remove(key);
							}
						}

						if (status.size())
						{
							ofstream fout(status_file);
							fout << status;
						}

						last_update = time(nullptr);

						write();

						cout << "Worklogs: " << count_updated << " updated and " << count_deleted << " deleted." << endl;
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
	else
		cout << "Please solve the following cases before using zeitkit pull:\n\n" << validate << endl;
}
