#ifndef CLIENT_H
#define CLIENT_H

#include <yaml-cpp/yaml.h>
#include <json.h>

#include <string>

class Client
{
	private:
		signed int id;
		std::string name;
		unsigned int rate;
		bool updated;
		bool deleted;

	public:
		Client() : id(0), rate(0), updated(false), deleted(false) {}
		Client(json_value* object);
		~Client() = default;

		signed int GetId() const { return id; }
		const std::string& GetName() const { return name; }
		unsigned int GetRate() const { return rate; }
		bool IsUpdated() const { return updated; }
		bool IsDeleted() const { return deleted; }

		void SetId(signed int id) { this->id = id; }
		void SetName(const std::string& name) { this->name = name; }
		void SetRate(unsigned int rate) { this->rate = rate; }

		std::string GetFileName();
		std::string GetIdString();
};

namespace YAML
{
	template<>
	struct convert<Client>
	{
		static Node encode(const Client& rhs)
		{
			Node node;
			node["id"] = rhs.GetId();
			node["name"] = rhs.GetName();
			node["hourly_rate_cents"] = rhs.GetRate();
			return node;
		}

		static bool decode(const Node& node, Client& rhs)
		{
			if (!node["id"] ||
				!node["name"] ||
				!node["hourly_rate_cents"])
					return false;

			rhs.SetId(node["id"].as<signed int>());
			rhs.SetName(node["name"].as<std::string>());
			rhs.SetRate(node["hourly_rate_cents"].as<unsigned int>());
			return true;
		}
	};
}

#endif
