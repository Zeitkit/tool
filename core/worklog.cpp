#include <core/worklog.h>
#include <utils/hash.h>

#include <cstring>

using namespace std;

Worklog::Worklog(unsigned int time_now, json_value* object) : updated(false), deleted(false)
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
				updated = !deleted && it->int_value > time_now;
				break;

			case Utils::hash("deleted_at_unixtime"):
				deleted = it->int_value > time_now;

				if (deleted)
					updated = false;
				break;

			default:
				break;
		}
	}
}

string Worklog::GetFileName()
{
	return GetIdString() + ".worklog";
}

string Worklog::GetIdString()
{
	if (!id)
		return "new";

	char buf[16];
	snprintf(buf, sizeof(buf), "%d", id);

	return buf;
}
