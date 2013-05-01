#include <core/zeitkit.h>
#include <utils/hash.h>

#include <iostream>
#include <unordered_map>
#include <cstring>
#include <cstdlib>

#ifdef __WIN32__
    #include <direct.h>
    #define getcwd _getcwd
#else
    #include <unistd.h>
#endif

using namespace std;

void collectArguments(unordered_map<string, const char*>& args, char const* const* argv, unsigned int argc)
{
	args.clear();

	while (argv[argc])
	{
		if (*argv[argc] == '-' && *(argv[argc] + 1) == '-' && *(argv[argc] + 2))
			args.emplace(argv[argc] + 2, argv[argc + 1]);

		++argc;
	}
}

int main(int argc, char** argv)
{
	if (argc < 2)
		return EXIT_SUCCESS;

	char* baseDirectory = getcwd(nullptr, 0);

	if (!baseDirectory)
		return EXIT_FAILURE;

	Zeitkit zeitkit(baseDirectory);
	free(baseDirectory);

	unordered_map<string, const char*> args;
	collectArguments(args, argv, 2);

	unsigned int op = Utils::hash(argv[1], strlen(argv[1]) + 1);

	try
	{
		switch (op)
		{
			case Utils::hash("init"):
			{
				zeitkit.init(args["mail"], args["password"], args.count("register"), args.count("force"));
				break;
			}

			case Utils::hash("auth"):
			{
				zeitkit.init(args["mail"], args["password"], false, true);
				break;
			}

			case Utils::hash("start"):
			{
				zeitkit.start(args.count("force"));
				break;
			}

			case Utils::hash("stop"):
			{
				zeitkit.stop(args.count("client") ? atoi(args["client"]) : 0, args["summary"], args["file"]);
				break;
			}

			case Utils::hash("status"):
			case Utils::hash("st"):
			{
				cout << zeitkit.status() << endl;
				break;
			}

			case Utils::hash("reset"):
			{
				zeitkit.reset(args.count("force"));
				break;
			}

			case Utils::hash("clients"):
			case Utils::hash("client"):
			{
				break;
			}

			case Utils::hash("log"):
			{
				if (argc < 3)
					return EXIT_SUCCESS;

				op = Utils::hash(argv[2], strlen(argv[2]) + 1);

				switch (op)
				{
					case Utils::hash("commit"):
					case Utils::hash("create"):
					{
						zeitkit.log_create(args.count("start") ? atoi(args["start"]) : 0, args.count("end") ? atoi(args["end"]) : 0, args.count("client") ? atoi(args["client"]) : 0, args["summary"], args["file"]);
						break;
					}

					default:
						cout << "Sorry, there is no command '" << argv[1] << " " << argv[2] << "'. Please try again." << endl;
				}

				break;
			}

			case Utils::hash("push"):
			{
				break;
			}

			case Utils::hash("pull"):
			{
				zeitkit.pull();
				break;
			}

			default:
				cout << "Sorry, there is no command '" << argv[1] << "'. Please try again." << endl;
		}
	}
	catch (exception& e)
	{
		cout << e.what() << endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
