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
const char* Zeitkit::fileStatusWorklog = "worklogs/.status";
const char* Zeitkit::fileNewWorklog = "worklogs/new.worklog";
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
		node = YAML::LoadFile(fileStatusWorklog);

		if (node.IsMap())
		{
			auto files = node.as<statusStruct>();

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

	if (result.size())
		result = result.substr(0, result.size() - 1);

	return result;
}

bool Zeitkit::is_worklog_open()
{
	YAML::Node node;

	try
	{
		YAML::LoadFile(fileNewWorklog);
	}
	catch (const YAML::BadFile& bad_file)
	{
		return false;
	}

	return true;
}

void Zeitkit::deploy(Worklog& worklog)
{
	YAML::Node status;

	try
	{
		status = YAML::LoadFile(fileStatusWorklog);
	}
	catch (const YAML::BadFile& bad_file) {}

	signed int new_id = -1;

	while (status[new_id])
		--new_id;

	worklog.SetId(new_id);

	string path = string(pathWorklogs) + "/" + worklog.GetFileName();

	YAML::Node node;
	node = worklog;
	ofstream worklog_(path.c_str());
	worklog_ << node;
	worklog_.flush();

	unsigned int crc = 0;
	Utils::crc32file(path.c_str(), &crc);
	status[new_id] = crc;

	ofstream fout(fileStatusWorklog);
	fout << status;
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

void Zeitkit::status()
{
	if (!initialized || auth_token.empty())
	{
		cout << "This directory has not yet been initialized." << endl;
		return;
	}

	string validate = validate_unchanged();
	bool new_worklog = is_worklog_open();

	if (validate.empty() && !new_worklog)
	{
		cout << "Zeitkit directory has been initialized and is clean." << endl;
		return;
	}

	if (!validate.empty())
		cout << validate << endl;

	if (new_worklog)
		cout << "A worklog has been started but not yet been closed." << endl;
}

void Zeitkit::reset(bool force)
{
	if (!initialized || auth_token.empty())
	{
		cout << "Please initialize this directory first: zeitkit init." << endl;
		return;
	}

	YAML::Node node;

	if ((!validate_unchanged().empty() || is_worklog_open()) && !force)
	{
		cout << "You have local changes which have not been pushed to remote which will be lost. Do you really want to continue? (y / n): ";

		char key;
		cin >> key;

		if (tolower(key) != 'y')
			return;
	}

	try
	{
		node = YAML::LoadFile(fileStatusWorklog);

		if (node.IsMap())
		{
			auto files = node.as<statusStruct>();

			for (const auto& file : files)
			{
				string file_ = string(pathWorklogs) + "/" + file.first + ".worklog";
				remove(file_.c_str());
				node.remove(file.first);
			}

			ofstream fout(fileStatusWorklog);
			fout << node;
		}
	}
	catch (const YAML::BadFile& bad_file) {}

	remove(fileNewWorklog);

	last_update = 0;

	write();
}

void Zeitkit::start(bool force)
{
	if (!initialized || auth_token.empty())
	{
		cout << "Please initialize this directory first: zeitkit init." << endl;
		return;
	}

	if (!force && is_worklog_open())
	{
		cout << "You've already started a worklog. Please close the current one with zeitkit stop first." << endl;
		return;
	}

	Worklog worklog(0, time(nullptr), 0, "");
	YAML::Node node;
	node = worklog;

	ofstream fout(fileNewWorklog);
	fout << node;
}

void Zeitkit::stop(unsigned int client_id, const char* summary)
{
	if (!initialized || auth_token.empty())
	{
		cout << "Please initialize this directory first: zeitkit init." << endl;
		return;
	}

	if (!is_worklog_open())
	{
		cout << "You didn't start a worklog yet. Please do so with zeitkit start first." << endl;
		return;
	}

	YAML::Node node = YAML::LoadFile(fileNewWorklog);
	Worklog worklog = node.as<Worklog>();
	remove(fileNewWorklog);

	create(worklog.GetStartTime(), time(nullptr), client_id, summary);
}

void Zeitkit::create(unsigned int start_time, unsigned int end_time, unsigned int client_id, const char* summary)
{
	if (!initialized || auth_token.empty())
	{
		cout << "Please initialize this directory first: zeitkit init." << endl;
		return;
	}

	if (!start_time)
	{
		cout << "Start time: ";
		cin >> start_time;
	}

	if (!end_time)
	{
		cout << "End time: ";
		cin >> end_time;
	}

	if (end_time <= start_time)
	{
		cout << "End time must be greater than the start time!" << endl;
		return;
	}

	if (!client_id)
	{
		cout << "Client ID: ";
		cin >> client_id;
	}

	string summary_;

	if (!summary)
	{
		cout << "Worklog summary: ";
		cin >> summary_;
	}
	else
		summary_ = summary;

	Worklog worklog(client_id, start_time, end_time, summary_);
	deploy(worklog);
}

void Zeitkit::push()
{

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

						try
						{
							status = YAML::LoadFile(fileStatusWorklog);
						}
						catch (const YAML::BadFile& bad_file) {}

						for (json_value* it = root->first_child; it; it = it->next_sibling)
						{
							Worklog worklog(it);
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
								if (!remove(file_name.c_str()))
									++count_deleted;

								if (status[key])
									status.remove(key);
							}
						}

						if (status.size())
						{
							ofstream fout(fileStatusWorklog);
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
