#include "../include/xhttp_server.hpp"
#include "../../xtest/include/xtest.hpp"

xtest_run;

XTEST_SUITE(query)
{
	XUNIT_TEST(query_test)
	{
		xhttper::query q("EIO=3&transport=polling&t=Lcm23ng&b64=5&sid=0.283746&uid=2938747464");

		xassert(q.get<int>("EIO") == 3);
		xassert(q.get("transport") == "polling");
		xassert(q.get("t") == "Lcm23ng");
		xassert(q.get<double>("sid") == double(0.283746));
		xassert(q.get<int64_t>("uid") == 2938747464L);
	}
}

XTEST_SUITE(xhttp_server)
{
	using namespace xhttp_server;

	void redis_session_test(request &req, response &rsp)
	{
		auto session =  req.get_session();
		xassert(session.set("hello", "world"));
		std::string result;
		xassert(session.get("hello",result));
		xassert(result == "world");
		rsp.done("hello world");
	}

	void upload_file_test(request &req, response &rsp)
	{
		uploader _uploader(req);
		_uploader.parse_request("");
		std::cout << std::endl;
		for(auto &itr: _uploader.get_field())
			std::cout << itr.first << ": " << itr.second << std::endl;
		for (auto &itr : _uploader.get_files())
			std::cout << itr.first << ": " << itr.second << std::endl;
		rsp.done("ok");
	}

	void download_file_test(request &req, response &)
	{
		downloader download_file(req);
		download_file.send_file("index.html");
	}
	void index(request &req, response &resp)
	{
		if (req.path() == "/")
		{
			resp.send_file("index.html");
			resp.done();
		}
		else
		{
			downloader download_file(req);
			if (!download_file.send_file(xutil::vfs::getcwd()() + req.path()))
			{
				resp.set_status(404);
				resp.done();
			}
		}
		
	}
	void filelist_test(request &req, response &resp)
	{
		auto path = req.url();
		if (path.back() == '/' || path.back() == '\\')
		{
			filelist fl(resp);
			return fl.resp_file_list(path);
		}
		downloader download_file(req);
		if (!download_file.send_file(xutil::vfs::getcwd()() +  path))
		{
			resp.set_status(404);
			resp.done();
		}
	}
	std::string async_get_str(int value)
	{
		return std::to_string(value);
	}

	void async_test(request &, response &resp)
	{
		auto value = async(async_get_str, 1);
		resp.done(value);
	}

	void hello (request &, response &resp)
	{
		resp.done("hello world");
	}

	XUNIT_TEST(regist)
	{
		xserver server;
		server.bind("0.0.0.0", 9001);
		//server.set_redis_addr("192.168.0.2",6379);
		server.regist(index);
		server.regist_run_before([&] {
			xhttp_server::init_async(server.get_proactor_pool().get_current_msgbox(), 1);
		});
		server.start();
		getchar();
	}
}
 
