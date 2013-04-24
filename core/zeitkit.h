#ifndef ZEITKIT_H
#define ZEITKIT_H

#include <string>

class Zeitkit
{
	private:
		static const char* fileZeitkit;
		static const char* pathWorklogs;
		static const char* pathClients;

		static const char* remoteAddr;
		static const unsigned int remotePort;
		static const char* queryLogin;
		static const char* globalHeaders[];

		std::string baseDirectory;
		bool initialized;
		std::string auth_token;

		Zeitkit(const Zeitkit&) = delete;
		Zeitkit& operator=(const Zeitkit&) = delete;

		void write();
		void authenticate(const std::string& input_mail, const std::string& input_pwd);

	public:
		Zeitkit(const char* baseDirectory);
		~Zeitkit();

		void init(const char* mail, const char* password, bool register_account, bool force);
};

#endif
