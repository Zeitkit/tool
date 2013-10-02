#include <core/zeitkit.h>
#include <core/worklog.h>
#include <core/client.h>
#include <utils/input.h>
#include <utils/checksum.h>
#include <utils/encode.h>
#include <utils/deleter.h>
#include <utils/parser.h>
#include <utils/http.h>

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
#include <memory>
#include <stdexcept>
#include <ctime>
#include <cstring>

using namespace std;

const char* Zeitkit::fileZeitkit = ".zeitkit";
const char* Zeitkit::fileStatusWorklog = "worklogs/.status";
const char* Zeitkit::fileStatusClient = "clients/.status";
const char* Zeitkit::fileNewWorklog = "worklogs/new.worklog";
const char* Zeitkit::pathWorklogs = "worklogs";
const char* Zeitkit::pathClients = "clients";

const char* Zeitkit::remoteAddr = "zeitkit.com";
const unsigned int Zeitkit::remotePort = 80;
const char* Zeitkit::queryLogin = "/sessions";
const char* Zeitkit::queryRegister = "/users";
const char* Zeitkit::queryWorklogs = "/worklogs";
const char* Zeitkit::queryClients = "/clients";

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
	string query = "{\"email\": \"" + Utils::json_encode(input_mail) + "\", \"password\": \"" + Utils::json_encode(input_pwd) + "\"}";

	Utils::http_request(remoteAddr, remotePort, globalHeaders, queryLogin, "POST", reinterpret_cast<const unsigned char*>(query.c_str()), query.size(), [this](signed int code, const string& result)
	{
		switch (code)
		{
			case happyhttp::UNAUTHORIZED:
				throw runtime_error("E-Mail / password is invalid. Do you want to create an account? Run zeitkit init --register.");

			case happyhttp::OK:
			{
				block_allocator allocator(1 << 10);
				unique_ptr<char, Utils::free_delete<char>> buffer(strdup(result.c_str()), Utils::free_delete<char>());
				json_value* root = Utils::json_parse(buffer.get(), &allocator);

				auth_token = root->first_child->string_value;
				write();
				break;
			}

			default:
				throw runtime_error(string("Unhandled HTTP status code: ") + Utils::inttostr(code));
		}
	});
}

void Zeitkit::register_account(const std::string& input_mail, const std::string& input_pwd)
{
	string query = "{\"email\": \"" + Utils::json_encode(input_mail) + "\", \"password\": \"" + Utils::json_encode(input_pwd) + "\"}";

	Utils::http_request(remoteAddr, remotePort, globalHeaders, queryRegister, "POST", reinterpret_cast<const unsigned char*>(query.c_str()), query.size(), [this](signed int code, const string& result)
	{
		block_allocator allocator(1 << 10);
		unique_ptr<char, Utils::free_delete<char>> buffer(strdup(result.c_str()), Utils::free_delete<char>());
		json_value* root = Utils::json_parse(buffer.get(), &allocator);

		switch (code)
		{
			case happyhttp::BAD_REQUEST:
			{
				string error = "Errors occured on registration, sorry!\n";

				if (root->first_child->type == JSON_ARRAY)
					for (json_value* it = root->first_child->first_child; it; it = it->next_sibling)
					{
						error += "\n";
						error += it->string_value;
					}

				throw runtime_error(error);
			}

			case happyhttp::CREATED:
			{
				auth_token = root->first_child->string_value;
				write();
				break;
			}

			default:
				throw runtime_error(string("Unhandled HTTP status code: ") + Utils::inttostr(code));
		}
	});
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

bool Zeitkit::is_valid_client(unsigned int id)
{
	YAML::Node node;

	try
	{
		node = YAML::LoadFile(fileStatusClient);

		if (node.IsMap())
		{
			auto files = node.as<statusStruct>();

			for (const auto& file : files)
			{
				string file_ = string(pathClients) + "/" + file.first + ".client";
				YAML::Node client = YAML::LoadFile(file_.c_str());

				if (client["id"].as<unsigned int>() == id)
					return true;
			}
		}
	}
	catch (const YAML::BadFile& bad_file) {}

	return false;
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
		throw runtime_error("Zeitkit has already been initialized in this directory. Use --force to perform the action anyway.");

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
			throw runtime_error("You entered an e-mail address. Please also give the password with --password yourpassword.");

		input_pwd = password;
	}

	if (register_account)
		this->register_account(input_mail, input_pwd);
	else
		authenticate(input_mail, input_pwd);
}

string Zeitkit::status()
{
	if (!initialized || auth_token.empty())
		throw runtime_error("This directory has not yet been initialized.");

	string validate = validate_unchanged();
	bool new_worklog = is_worklog_open();

	if (validate.empty() && !new_worklog)
		return "Zeitkit directory has been initialized and is clean.";

	if (new_worklog)
	{
		if (!validate.empty())
			validate += "\n";

		validate += "A worklog has been started but not yet been closed.";
	}

	return validate;
}

string Zeitkit::clients()
{
	if (!initialized || auth_token.empty())
		throw runtime_error("This directory has not yet been initialized.");

	YAML::Node node;

	string result;

	try
	{
		node = YAML::LoadFile(fileStatusClient);

		if (node.IsMap())
		{
			auto files = node.as<statusStruct>();

			for (const auto& file : files)
			{
				string file_ = string(pathClients) + "/" + file.first + ".client";
				YAML::Node client = YAML::LoadFile(file_.c_str());
				result += "[" + client["id"].as<string>() + "]\t" + client["name"].as<string>() + " (" + client["hourly_rate_cents"].as<string>() + " cents)\n";
			}
		}
	}
	catch (const YAML::BadFile& bad_file) {}

	if (!result.empty())
		result = result.substr(0, result.size() - 1);

	return result;
}

void Zeitkit::reset(bool force)
{
	if (!initialized || auth_token.empty())
		throw runtime_error("Please initialize this directory first: zeitkit init.");

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

		node = YAML::LoadFile(fileStatusClient);

		if (node.IsMap())
		{
			auto files = node.as<statusStruct>();

			for (const auto& file : files)
			{
				string file_ = string(pathClients) + "/" + file.first + ".client";
				remove(file_.c_str());
				node.remove(file.first);
			}

			ofstream fout(fileStatusClient);
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
		throw runtime_error("Please initialize this directory first: zeitkit init.");

	if (!force && is_worklog_open())
		throw runtime_error("You've already started a worklog. Please close the current one with zeitkit stop first.");

	Worklog worklog(0, time(nullptr), 0, "");
	YAML::Node node;
	node = worklog;

	ofstream fout(fileNewWorklog);
	fout << node;
}

void Zeitkit::stop(unsigned int client_id, const char* summary, const char* file)
{
	if (!initialized || auth_token.empty())
		throw runtime_error("Please initialize this directory first: zeitkit init.");

	if (!is_worklog_open())
		throw runtime_error("You didn't start a worklog yet. Please do so with zeitkit start first.");

	YAML::Node node = YAML::LoadFile(fileNewWorklog);
	Worklog worklog = node.as<Worklog>();

	log_create(worklog.GetStartTime(), time(nullptr), client_id, summary, file);
	remove(fileNewWorklog);
}

void Zeitkit::log_create(unsigned int start_time, unsigned int end_time, unsigned int client_id, const char* summary, const char* file)
{
	if (!initialized || auth_token.empty())
		throw runtime_error("Please initialize this directory first: zeitkit init.");

	if (!start_time)
	{
		cout << "Start time (YYYY-MM-DD-HH:MM:SS): ";
		string start_time_;
		getline(cin, start_time_);
		start_time = Time::LocaltimeToUnixtime(start_time_);
	}

	if (!end_time)
	{
		cout << "End time (YYYY-MM-DD-HH:MM:SS): ";
		string end_time_;
		getline(cin, end_time_);
		end_time = Time::LocaltimeToUnixtime(end_time_);
	}

	if (end_time <= start_time)
		throw runtime_error("End time must be greater than the start time!");

	if (!client_id)
	{
		cout << clients() << "\nClient ID: ";
		cin >> client_id;
	}

	if (!is_valid_client(client_id))
		throw runtime_error("The client ID is not valid. Get a list of your clients with zeitkit clients.");

	string summary_;

	if (!file)
	{
		if (!summary)
		{
			cout << "Worklog summary: ";
			cin.ignore();
			getline(cin, summary_);
		}
		else
			summary_ = summary;
	}
	else
	{
		ifstream fin(file, ios::in | ios::binary);

		if (!fin)
			throw runtime_error("File could not be opened.");

		summary_ = Utils::get_file_contents(fin);
	}

	Worklog worklog(client_id, start_time, end_time, summary_);
	deploy(worklog);
}

void Zeitkit::push()
{

}

void Zeitkit::pull()
{
	if (!initialized || auth_token.empty())
		throw runtime_error("Please initialize this directory first: zeitkit init.");

	string validate = validate_unchanged();

	if (validate.empty())
	{
#ifdef __WIN32__
		mkdir(pathWorklogs);
		mkdir(pathClients);
#else
		constexpr auto DIR_MODE = S_IRWXU | S_IRGRP |  S_IXGRP | S_IROTH | S_IXOTH;
		mkdir(pathWorklogs, DIR_MODE);
		mkdir(pathClients, DIR_MODE);
#endif
		string query = string("{\"updated_since\": ") + Utils::inttostr(last_update) + ", \"access_token\": \"" + Utils::json_encode(auth_token) + "\"}";

		unsigned int new_last_update = time(nullptr);

		Utils::http_request(remoteAddr, remotePort, globalHeaders, queryWorklogs, "GET", reinterpret_cast<const unsigned char*>(query.c_str()), query.size(), [this](signed int code, const string& result)
		{
			switch (code)
			{
				case happyhttp::UNAUTHORIZED:
					throw runtime_error("Your authentication has expired. Please use zeitkit auth to re-authenticate with the server.");

				case happyhttp::OK:
				{
					block_allocator allocator(1 << 10);
					unique_ptr<char, Utils::free_delete<char>> buffer(strdup(result.c_str()), Utils::free_delete<char>());
					json_value* root = Utils::json_parse(buffer.get(), &allocator);

					if (root->type == JSON_ARRAY)
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

						cout << "Worklogs: " << count_updated << " updated and " << count_deleted << " deleted." << endl;
					}

					break;
				}

				default:
					throw runtime_error(string("Unhandled HTTP status code: ") + Utils::inttostr(code));
			}
		});

		Utils::http_request(remoteAddr, remotePort, globalHeaders, queryClients, "GET", reinterpret_cast<const unsigned char*>(query.c_str()), query.size(), [this](signed int code, const string& result)
		{
			switch (code)
			{
				case happyhttp::UNAUTHORIZED:
					throw runtime_error("Your authentication has expired. Please use zeitkit auth to re-authenticate with the server.");

				case happyhttp::OK:
				{
					block_allocator allocator(1 << 10);
					unique_ptr<char, Utils::free_delete<char>> buffer(strdup(result.c_str()), Utils::free_delete<char>());
					json_value* root = Utils::json_parse(buffer.get(), &allocator);

					if (root->type == JSON_ARRAY)
					{
						unsigned int count_updated = 0;
						unsigned int count_deleted = 0;

						YAML::Node status;

						try
						{
							status = YAML::LoadFile(fileStatusClient);
						}
						catch (const YAML::BadFile& bad_file) {}

						for (json_value* it = root->first_child; it; it = it->next_sibling)
						{
							Client client(it);
							string file_name = string(pathClients) + "/" + client.GetFileName();
							string key = client.GetIdString();

							if (client.IsUpdated())
							{
								YAML::Node node;
								node = client;

								ofstream fout(file_name);
								fout << node;
								fout.flush();

								++count_updated;

								unsigned int crc = 0;
								Utils::crc32file(file_name.c_str(), &crc);
								status[key] = crc;
							}
							else if (client.IsDeleted())
							{
								if (!remove(file_name.c_str()))
									++count_deleted;

								if (status[key])
									status.remove(key);
							}
						}

						if (status.size())
						{
							ofstream fout(fileStatusClient);
							fout << status;
						}

						cout << "Clients: " << count_updated << " updated and " << count_deleted << " deleted." << endl;
					}

					break;
				}

				default:
					throw runtime_error(string("Unhandled HTTP status code: ") + Utils::inttostr(code));
			}
		});

		last_update = new_last_update;

		write();
	}
	else
		throw runtime_error(string("Please solve the following cases before using zeitkit pull:\n\n") + validate);
}
