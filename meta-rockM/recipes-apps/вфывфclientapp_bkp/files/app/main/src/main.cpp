#include "crow.h"

int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([](){
        return "Hello world";
    });

	CROW_ROUTE(app, "/json")
	([]{
	    crow::json::wvalue x({{"message", "Hello, World!"}});
    	x["message2"] = "Hello, World.. Again!";
    	return x;
	});

    app.port(18080).multithreaded().run();
}
