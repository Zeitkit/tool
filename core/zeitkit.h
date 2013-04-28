#ifndef ZEITKIT_H
#define ZEITKIT_H

#include <string>
#include <map>

class Zeitkit
{
	private:
		typedef std::map<std::string, unsigned int> statusStruct;

		static const char* fileZeitkit;
		static const char* fileStatusWorklog;
		static const char* fileNewWorklog;
		static const char* pathWorklogs;
		static const char* pathClients;

		static const char* remoteAddr;
		static const unsigned int remotePort;
		static const char* queryLogin;
		static const char* queryRegister;
		static const char* queryWorklogs;

		static const char* globalHeaders[];

		std::string baseDirectory;
		bool initialized;
		std::string auth_token;
		unsigned int last_update;

		Zeitkit(const Zeitkit&) = delete;
		Zeitkit& operator=(const Zeitkit&) = delete;

		void write();
		void authenticate(const std::string& input_mail, const std::string& input_pwd);
		void register_account(const std::string& input_mail, const std::string& input_pwd);
		std::string validate_unchanged();
		bool is_worklog_open();

	public:
		Zeitkit(const char* baseDirectory);
		~Zeitkit();

		void init(const char* mail, const char* password, bool register_account, bool force);
		void status();
		void reset(bool force);
		void pull();

};

#endif
