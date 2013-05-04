#include <core/worklog.h>
#include <utils/hash.h>
#include <utils/encode.h>

#include <cstring>

using namespace std;

Worklog::Worklog(json_value* object) : updated(false), deleted(false)
{
	for (json_value* it = object->first_child; it; it = it->next_sibling)
	{
		unsigned int hash = Utils::hash(it->name, strlen(it->name) + 1);

		switch (hash)
		{
			case Utils::hash("id"):
				this->id = it->int_value;
				break;

			case Utils::hash("client_id"):
				this->client_id = it->int_value;
				break;

			case Utils::hash("start_time_unixtime"):
				this->start_time = it->int_value;
				break;

			case Utils::hash("end_time_unixtime"):
				this->end_time = it->int_value;
				break;

			case Utils::hash("summary"):
				this->summary = it->string_value;
				break;

			case Utils::hash("updated_at_unixtime"):
				updated = !deleted;
				break;

			case Utils::hash("deleted_at_unixtime"):
				if (it->int_value)
				{
					deleted = true;
					updated = false;
				}
				break;

			default:
				break;
		}
	}
}

Worklog::Worklog(unsigned int client_id, unsigned int start_time, unsigned int end_time, const string& summary) : id(0), client_id(client_id), start_time(start_time), end_time(end_time), summary(summary), updated(false), deleted(false)
{

}

string Worklog::GetFileName()
{
	return GetIdString() + ".worklog";
}

string Worklog::GetIdString()
{
	if (!id)
		return "new";

	return Utils::inttostr(id);
}
