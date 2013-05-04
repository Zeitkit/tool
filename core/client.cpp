#include <core/client.h>
#include <utils/hash.h>
#include <utils/encode.h>

#include <cstring>

using namespace std;

Client::Client(json_value* object) : updated(false), deleted(false)
{
	for (json_value* it = object->first_child; it; it = it->next_sibling)
	{
		unsigned int hash = Utils::hash(it->name, strlen(it->name) + 1);

		switch (hash)
		{
			case Utils::hash("id"):
				this->id = it->int_value;
				break;

			case Utils::hash("name"):
				this->name = it->string_value;
				break;

			case Utils::hash("hourly_rate_cents"):
				this->rate = it->int_value;
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

string Client::GetFileName()
{
	return GetIdString() + ".client";
}

string Client::GetIdString()
{
	if (!id)
		return "new";

	return Utils::inttostr(id);
}
