#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

using namespace std;

typedef vector<size_t> VideoStorage;

struct Endpoint
{
    Endpoint(size_t latency_, size_t num_connections_)
    :
            _latency(latency_),
            _num_connections(num_connections_),
            _cache_connections(_num_connections),
            _cache_latency(_num_connections),
            _connections_sorted(_num_connections),
            _cmp(*this)
    {}



    void sort_connections();
    bool cmp(size_t i, size_t j);

public:
    struct cmp_class
    {
        cmp_class(Endpoint & parent_) : _parent(parent_) {};

        bool operator() (size_t i, size_t j)
        {
           return (_parent._latency - _parent._cache_latency[i]) > (_parent._latency - _parent._cache_latency[j]);
        };

        Endpoint & _parent;
    };

    size_t _latency;
    size_t _num_connections;
    vector<size_t> _cache_connections;
    vector<size_t> _cache_latency;
    vector<size_t> _connections_sorted;
    cmp_class _cmp;


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
    void write_submission_file(char const * file);
    void stupid_algorithm();
    bool cmp_requests(size_t i, size_t j);
    bool cmp_requests2(size_t i, size_t j);
    void sort_requests(vector<vector<size_t> > & req_by_endpt);
    bool add_to_server(size_t server_id, size_t video_id);

    struct cmp_class2
    {
        cmp_class2(VideoStream & parent_) : _parent(parent_) {};

        VideoStream & _parent;

        bool operator() (size_t i, size_t j)
        {
           Request & first = _parent._requests[i];
           Endpoint & first_endpt = _parent._endpts[first._endpt_id];
           if (first_endpt._connections_sorted.size() == 0)
           {
              return false;
           }
           Request & second = _parent._requests[j];
           Endpoint & second_endpt = _parent._endpts[second._endpt_id];
           if (second_endpt._connections_sorted.size() == 0)
           {
              return true;
           }
           size_t first_gain = first_endpt._latency - _parent._endpt_server_latency[first._endpt_id][first_endpt._connections_sorted[0]];
           size_t second_gain = second_endpt._latency - _parent._endpt_server_latency[second._endpt_id][second_endpt._connections_sorted[0]];
           return (first._num_requests * first_gain) > (second._num_requests * second_gain);
        }

    };

private:
    size_t _num_vids;
    size_t _num_endpts;
    size_t _num_requests;
    size_t _num_servers;
    size_t _capacity;
    vector<size_t> _vid_size;
    vector<Endpoint> _endpts;
    vector<Request> _requests;
    vector<VideoStorage> _vids_stored;
    vector<vector<size_t> > _endpt_server_latency;
    cmp_class2 _cmp;
};

int main(int argc, char ** argv) {
   if (argc >= 2)
   {
      VideoStream vidstream(argv[1]);
      vidstream.stupid_algorithm();
      string input(argv[1]);
      input.append("_sol");
      vidstream.write_submission_file(input.c_str());
      return 0;
   }
   return 1;
}

VideoStream::VideoStream(char *file) : _cmp(*this)
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

   //cout << "Basic" << endl;

   //cout << _num_vids << " " << _num_endpts << " " << _num_requests << " " << _num_servers << " " << _capacity << endl;

   getline(filestream, line);
   stringstream ss2(line);

   //cout << "Vid sizes" << endl;

   _vid_size.reserve(_num_vids);
   size_t tmp;
   for (size_t i = 0; i < _num_vids; ++i)
   {
      ss2 >> tmp;
      //cout << tmp << endl;
      _vid_size.push_back(tmp);
   }

   //cout << "endpts" << endl;

   size_t latency, num_connections;
   size_t cache_id, cache_latency;
   _endpts.reserve(_num_endpts);
   for (size_t i = 0; i < _num_endpts; ++i)
   {
      //cout << "new endpt" << endl;
      getline(filestream, line);
      ss2.str(line);
      ss2.clear();
      ss2 >> latency;
      ss2 >> num_connections;
      //cout << "   " << latency << " " << num_connections << endl;
      Endpoint new_endpt(latency, num_connections);
      for (size_t j = 0; j < num_connections; ++j)
      {
         //cout << "   new connection" << endl;
         getline(filestream, line);
         ss2.str(line);
         ss2.clear();
         ss2 >> cache_id;
         ss2 >> cache_latency;
         //cout << "      " << cache_id << " " << cache_latency << endl;
         new_endpt._cache_connections[j] = cache_id;
         new_endpt._cache_latency[j] = cache_latency;
      }

      _endpts.push_back(new_endpt);
   }

   //cout << "requests" << endl;
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
      //cout << vid_id << " " << endpt_id << " " << num_requests << endl;
      Request new_req(vid_id, endpt_id, num_requests);
      _requests.push_back(new_req);
   }

   cout << "Reading input succesful" << endl;
}

void VideoStream::write_submission_file(char const * file)
{
   ofstream output_file;
   output_file.open(file);

   output_file << _vids_stored.size() << endl;

   for(size_t i = 0; i < _vids_stored.size(); i++)
   {
      VideoStorage const vids = _vids_stored[i];
      output_file << i;
      cout << vids.size() << " videos on server " << i << endl;
      for (size_t j = 0; j < vids.size(); ++j)
      {
         output_file << " " << vids[j];
      }
      output_file << endl;
   }

   output_file.close();
}

bool Endpoint::cmp(size_t i, size_t j)
{
   return (_latency - _cache_latency[i]) > (_latency - _cache_latency[j]);
}

void Endpoint::sort_connections()
{
   cout << "Number of connections to this endpoint: " << _cache_connections.size() << endl;
   _connections_sorted.assign(_cache_connections.begin(), _cache_connections.end());
   sort(_connections_sorted.begin(), _connections_sorted.end(), _cmp);
}

bool VideoStream::cmp_requests(size_t i, size_t j)
{
   return _requests[i]._num_requests > _requests[j]._num_requests;
}

bool VideoStream::cmp_requests2(size_t i, size_t j)
{
   Request & first = _requests[i];
   Endpoint & first_endpt = _endpts[first._endpt_id];
   if (first_endpt._connections_sorted.size() == 0)
   {
      return false;
   }
   Request & second = _requests[j];
   Endpoint & second_endpt = _endpts[second._endpt_id];
   if (second_endpt._connections_sorted.size() == 0)
   {
      return true;
   }
   size_t first_gain = first_endpt._latency - _endpt_server_latency[first._endpt_id][first_endpt._connections_sorted[0]];
   size_t second_gain = second_endpt._latency - _endpt_server_latency[second._endpt_id][second_endpt._connections_sorted[0]];
   return (first._num_requests * first_gain) > (second._num_requests * second_gain);
}

/*void VideoStream::sort_requests(vector<vector<size_t> > & reqs_by_endpt)
{
   for (size_t i = 0; i < reqs_by_endpt.size(); ++i)
   {
      sort(reqs_by_endpt[i].begin(), reqs_by_endpt[i].end(), cmp_requests);
   }
}*/

bool VideoStream::add_to_server(size_t endpt_id, size_t video_id)
{
   Endpoint & endpt = _endpts[endpt_id];
   if (endpt._connections_sorted.size() > 0) {
      size_t idx = 0;
      while (idx < endpt._connections_sorted.size()) {
         size_t server_id = endpt._connections_sorted[idx];
         VideoStorage & vids_on_server = _vids_stored[server_id];
         bool contains_vid = false;
         size_t space_occupied = 0;
         for (size_t i = 0; i < vids_on_server.size(); ++i) {
            if (vids_on_server[i] == video_id) {
               contains_vid = true;
               break;
            }
            space_occupied += _vid_size[vids_on_server[i]];
         }
         if (not contains_vid) {
            if (space_occupied + _vid_size[video_id] <= _capacity) {
               vids_on_server.push_back(video_id);
               return true;
            }
         } else {
            return true;
         }
         ++idx;
      }
   }
   else
   {
      return false;
   }
}

void VideoStream::stupid_algorithm()
{
   cout << "Number of endpoints: " << _num_endpts << endl;
   for (size_t i = 0; i < _num_endpts; ++i)
   {
      cout << "endpoint " << i << endl;
      _endpts[i].sort_connections();
   }
   cout << "Connections sorted" << endl;

   /*vector<vector<size_t> > reqs_by_endpt(_num_endpts);
   for (size_t i = 0; i < _num_endpts; ++i)
   {
      reqs_by_endpt[i].reserve(_num_requests / _num_endpts);
      for (size_t j = 0; j < _num_requests; ++j)
      {
         if (_requests[j]._endpt_id == i)
         {
            reqs_by_endpt[i].push_back(j);
         }
      }
   }

   sort_requests(reqs_by_endpt);*/



   _endpt_server_latency.reserve(_num_endpts);
   vector<size_t> zeroes(_num_servers, 0);
   for (size_t i = 0; i < _num_endpts; ++i)
   {
      _endpt_server_latency.push_back(zeroes);
      Endpoint cur_endpt = _endpts[i];
      for (size_t j = 0; j < cur_endpt._num_connections; ++j)
      {
         _endpt_server_latency[i][cur_endpt._cache_connections[j]] = cur_endpt._cache_latency[j]; 
      }
   }

   cout << "Generated latency matrix" << endl;

   vector<size_t> sorted_req_ids(_num_requests);
   for (size_t i = 0; i < _num_requests; ++i)
   {
      sorted_req_ids[i] = i;
   }

   sort(sorted_req_ids.begin(), sorted_req_ids.end(), _cmp);

   cout << "Sorted requests" << endl;


   VideoStorage empty;
   _vids_stored.assign(_num_servers, empty);

   cout << "Set up video storage" << endl;

   /*bool done = false;
   while (not done)
   {

   }*/

   for (size_t i = 0; i < _num_requests; ++i)
   {
      Request & cur_req = _requests[sorted_req_ids[i]];
      Endpoint & cur_endpt = _endpts[cur_req._endpt_id];
      if (cur_endpt._connections_sorted.size() > 0);
      {
         add_to_server(cur_req._endpt_id, cur_req._vid_id);
      }

   }

}

