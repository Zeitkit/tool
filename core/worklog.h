#ifndef WORKLOG_H
#define WORKLOG_H

#include <yaml-cpp/yaml.h>
#include <json.h>

#include <string>

class Worklog
{
	private:
		unsigned int id;
		unsigned int client_id;
		unsigned int start_time;
		unsigned int end_time;
		std::string summary;
		bool updated;
		bool deleted;

	public:
		Worklog(json_value* object);
		~Worklog() = default;

		unsigned int GetId() const { return id; }
		unsigned int GetClientId() const { return client_id; }
		unsigned int GetStartTime() const { return start_time; }
		unsigned int GetEndTime() const { return end_time; }
		const std::string& GetSummary() const { return summary; }
		bool IsUpdated() { return updated; }
		bool IsDeleted() { return deleted; }

		void SetId(unsigned int id) { this->id = id; }
		void SetClientId(unsigned int client_id) { this->client_id = client_id; }
		void SetStartTime(unsigned int start_time) { this->start_time = start_time; }
		void SetEndTime(unsigned int end_time) { this->end_time = end_time; }
		void SetSummary(const std::string& summary) { this->summary = summary; }

		std::string GetFileName();
		std::string GetIdString();
};

namespace YAML
{
	template<>
	struct convert<Worklog>
	{
		static Node encode(const Worklog& rhs)
		{
			Node node;
			node["id"] = rhs.GetId();
			node["client_id"] = rhs.GetClientId();
			node["start_time"] = rhs.GetStartTime();
			node["end_time"] = rhs.GetEndTime();
			node["summary"] = rhs.GetSummary();
			return node;
		}

		static bool decode(const Node& node, Worklog& rhs)
		{
			if (!node["id"] ||
				!node["client_id"] ||
				!node["start_time"] ||
				!node["end_time"] ||
				!node["summary"])
					return false;

			rhs.SetId(node["id"].as<unsigned int>());
			rhs.SetClientId(node["client_id"].as<unsigned int>());
			rhs.SetStartTime(node["start_time"].as<unsigned int>());
			rhs.SetEndTime(node["end_time"].as<unsigned int>());
			rhs.SetSummary(node["summary"].as<std::string>());
			return true;
		}
	};
}

#endif
