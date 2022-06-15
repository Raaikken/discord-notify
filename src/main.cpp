#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <time.h>
#include "dpp/intents.h"
#include "unistd.h"

#include "dpp/dpp.h"
#include "nlohmann/json.hpp"

using namespace std;
using namespace dpp;
using namespace nlohmann;

struct twitchData {
	string game_id;
	string game_name;
	string id;
	int is_mature;
	string language;
	string started_at;
	string tag_ids;
	string thumbnail_url;
	string title;
	string type;
	string user_id;
	string user_login;
	string user_name;
	int viewer_count;
};
enum StreamStatus {
	Live,
	Offline,
};

string exec(const char*);
StreamStatus getStreamStatus();
void constructMessage(string&);
message sendNotification(cluster&);
void logLocal(string);

twitchData streamData;
StreamStatus previousStreamStatus = StreamStatus::Offline;

string exec(const char* cmd) {
	array<char, 128> buffer;
	string result;
	unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
	if (!pipe) {
		throw runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}

	return result;
}
StreamStatus getStreamStatus() {
	string command = "";
	command += "twitch api get streams -q user_login=";
	command += getenv("TWITCH_USER_NAME");
	auto response = json::parse(exec(command.c_str()));
	json empty;

	if(response["data"][0] != empty) {
		streamData.game_id = response["data"][0]["game_id"];
		streamData.game_name = response["data"][0]["game_name"];
		streamData.id = response["data"][0]["id"];
		streamData.is_mature = response["data"][0]["is_mature"];
		streamData.language = response["data"][0]["language"];
		streamData.started_at = response["data"][0]["started_at"];
		// streamData.tag_ids = response["data"][0]["tag_ids"];
		streamData.thumbnail_url = response["data"][0]["thumbnail_url"];
		streamData.title = response["data"][0]["started_at"];
		streamData.type = response["data"][0]["type"];
		streamData.user_id = response["data"][0]["user_id"];
		streamData.user_login = response["data"][0]["user_login"];
		streamData.user_name = response["data"][0]["user_name"];
		streamData.viewer_count = response["data"][0]["viewer_count"];

		return StreamStatus::Live;
	}
	else {
		return StreamStatus::Offline;
	}
}
void constructMessage(string& msg) {
	logLocal("Consturcting the message.");
	msg = "";
	msg += "@everyone \n";
	msg += "🤖 ";
	// Generate Beeps and Boops
	srand(time(NULL));
	for (size_t i = 0; i < 3; i++) {
		msg += rand() % 2 == 1 ? "BEEP " : "BOOP ";
	}
	msg += "🤖\n\n";

	// Fetch text
	msg += streamData.user_name + " started streaming " + streamData.game_name + "!\n";

	// Finally add the link
	msg += "\nhttps://twitch.tv/" + streamData.user_login;
	logLocal("Message constructed.");
}
message sendNotification(cluster& bot) {
	string msg = "";

	constructMessage(msg);
	message notificationMsg(strtol(getenv("DISCORD_CHANNEL"), NULL, 10), msg);
	notificationMsg.set_allowed_mentions(false, false, true, false, vector<snowflake>(), vector<snowflake>());

	bot.message_create(notificationMsg);

	return notificationMsg;
}
void logLocal(string msg) {
	time_t theTime = time(NULL);
	struct tm *aTime = localtime(&theTime);
	cout << "[" << aTime->tm_hour << ":" << aTime->tm_min << ":" << aTime->tm_sec << "] " << msg << endl;
}
void callback(const confirmation_callback_t msgErr){
	cout << msgErr.get_error().message << endl;
}
int main(){
	logLocal("Hello, World!");

	cluster bot(getenv("DISCORD_TOKEN"));

	message notificationMessage;
	
	bot.on_ready([&bot](const ready_t & event) {
		logLocal(("Logged in as %s", bot.me.username));
	});

	bot.on_message_create([&bot](const message_create_t & event) {
		if(event.msg.content == "~ping") {
			bot.message_create(message(event.msg.channel_id, "Pong!"));
		}
	});

	bot.start(true);

	int secondsTillMinute = 0;

	while(true) {
		secondsTillMinute = 60 - (time(NULL) % 60);

		cout << secondsTillMinute << endl;

		if(secondsTillMinute == 60) {
			logLocal("Fetching stream status...");
			if(getStreamStatus() == StreamStatus::Live && previousStreamStatus == StreamStatus::Offline) {
				logLocal("Stream has gone live.");
				notificationMessage = sendNotification(bot);
				cout << "Notification MSG Info\nID: " << notificationMessage.id << "\nChannel: " << notificationMessage.channel_id << endl;
				previousStreamStatus = StreamStatus::Live;
			}
			else if(getStreamStatus() == StreamStatus::Offline && previousStreamStatus == StreamStatus::Live) {
				logLocal("Stream has gone offline.");
				bot.message_delete(notificationMessage.id, notificationMessage.channel_id, callback);
				sleep(10);
				break;
			}
			else{
				logLocal("Stream status has not changed.");
			}
		}
		else if(secondsTillMinute <= 5) {
			sleep(1);
		}
		else {
			sleep(secondsTillMinute - 5);
		}
	}

	return 0;
}