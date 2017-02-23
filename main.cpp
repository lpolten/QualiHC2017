#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

struct Endpoint
{
    Endpoint(size_t latency_, size_t num_connections_) : _latency(latency_), _num_connections(num_connections_), _cache_connections(_num_connections), _cache_latency(_num_connections) {}

    size_t _latency;
    size_t _num_connections;
    vector<size_t> _cache_connections;
    vector<size_t> _cache_latency;
};

struct Request
{
    Request(size_t vid_id_, size_t endpt_id_, size_t num_requests_) : _vid_id(vid_id_), _endpt_id(endpt_id_), _num_requests(num_requests_) {}

    size_t _vid_id;
    size_t _endpt_id;
    size_t _num_requests;
};

class VideoStream
{
public:
    VideoStream(char * file);

private:
    size_t _num_vids;
    size_t _num_endpts;
    size_t _num_requests;
    size_t _num_servers;
    size_t _capacity;
    vector<size_t> _vid_size;
    vector<Endpoint> _endpts;
    vector<Request> _requests;
};

int main(int argc, char ** argv) {
   if (argc >= 2)
   {
      VideoStream vidstream(argv[1]);
      return 0;
   }
   return 1;
}

VideoStream::VideoStream(char *file)
{
   std::fstream filestream(file);

   std::string line;
   getline(filestream, line);
   std::stringstream ss(line);

   ss >> _num_vids;
   ss >> _num_endpts;
   ss >> _num_requests;
   ss >> _num_servers;
   ss >> _capacity;

   getline(filestream, line);
   stringstream ss2(line);

   _vid_size.reserve(_num_vids);
   size_t tmp;
   for (size_t i = 0; i < _num_vids; ++i)
   {
      ss2 >> tmp;
      _vid_size.push_back(tmp);
   }

   size_t latency, num_connections;
   size_t cache_id, cache_latency;
   _endpts.reserve(_num_endpts);
   for (size_t i = 0; i < _num_endpts; ++i)
   {
      getline(filestream, line);
      ss2.str(line);
      ss2.clear();
      ss2 >> latency;
      ss2 >> num_connections;
      Endpoint new_endpt(latency, num_connections);
      for (size_t j = 0; j < num_connections; ++j)
      {
         getline(filestream, line);
         ss2.str(line);
         ss2.clear();
         ss2 >> cache_id;
         ss2 >> cache_latency;
         new_endpt._cache_connections[j] = cache_id;
         new_endpt._cache_latency[j] = cache_latency;
      }
      
      _endpts.push_back(new_endpt);
   }

   size_t vid_id, endpt_id, num_requests;
   _requests.reserve(_num_requests);
   for (size_t i = 0; i < _num_requests; ++i)
   {
      getline(filestream, line);
      ss2.str(line);
      ss2.clear();
      ss2 >> vid_id;
      ss2 >> endpt_id;
      ss2 >> num_requests;
      Request new_req(vid_id, endpt_id, num_requests);
      _requests.push_back(new_req);
   }
}

