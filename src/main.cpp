#include <cstdlib>
#include <future>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <format>
#include <string>
#include <thread>


typedef struct reportItem {
  bool success;
  std::string name;
  std::string version;
  std::string error;
  std::string full_error;
  std::string full_cmd;
  int status_code;
  bool failed_on_add;
  bool failed_on_run;
} reportItem;

void toLower(std::string& str){
  for(auto& i : str){
    i = std::tolower(i);
  }
}

bool checkOutForError(std::string& out){
  toLower(out);
  if(out.find("] error") != std::string::npos || out.find("error:") != std::string::npos || out.find("failed") != std::string::npos){
    return true;
  }
  return false;
}

typedef struct cmdReturn {
  std::string out;
  int status;
} cmdReturn;
cmdReturn exec(std::string cmd, int thread_id){
  std::string out;
  std::string thread_id_str = std::to_string(thread_id);
  int status = std::system((cmd + " &> /tmp/out"+thread_id_str+".txt").c_str());
  std::ifstream out_file;
  try{
    out_file = std::ifstream("/tmp/out.txt");
  }catch(std::exception& e){
    std::cout << e.what() << std::endl;
    exit(1);
  }
  for(std::string line; std::getline(out_file, line);){
    out += line;
  }
  out_file.close();
  return {out, status};
}

void addEntry(nlohmann::json &report, reportItem item){
  nlohmann::json entry;
  entry["success"] = item.success;
  entry["name"] = item.name;
  entry["version"] = item.version;
  entry["error"] = item.error;
  entry["failed_on_add"] = item.failed_on_add;
  entry["failed_on_run"] = item.failed_on_run;
  entry["full_error"] = item.full_error;
  entry["full_cmd"] = item.full_cmd;
  entry["status_code"] = item.status_code;
  std::system(("touch /report/" + item.name + "-error.json").c_str());
  std::ofstream out;
  try{
    out.open(("/report/" + item.name + "-error.json"));
    out << std::setw(4) << entry.dump(4) << std::endl;
  }catch(std::exception& e){
    std::cout << e.what() << std::endl;
    exit(1);
  }
  report.push_back(entry);
  out.close();
}
reportItem cmakerRunner(nlohmann::json package,int thread_id){
    reportItem item{};
    if(package["versions"].size() == 0){
      std::cout << "Package has no versions: " << (std::string)package["name"] << std::endl;
      item.error = "Package has no versions";
      item.failed_on_add = false;
      item.failed_on_run = false;
      item.full_cmd = "";
      item.status_code = 69;
      item.name = "no-version";
      return item;
    }

    if(package["name"].is_null()){
      std::cout << "Package has no name" << std::endl;
      item.error = "Package has no name";
      item.failed_on_add = false;
      item.failed_on_run = false;
      item.full_cmd = "";
      item.status_code = 69;
      item.name = "no-name";
      return item;
    }

    item.name = (std::string)package["name"];
    if(package["versions"].size() > 1){
      item.version = (std::string)package["versions"][1];
    }else{
      item.version = (std::string)package["versions"][0];
    }
    std::string full_cmd;
    std::string path = "/tmp/" + (std::string)package["name"] + "-test";
    std::filesystem::create_directory(path);
    std::string cd_cmd = "cd " + path;
    std::string cmaker_init_cmd = "cmaker init -y";
    std::string cmaker_add_dep_cmd = "cmaker add dep -l -e " + (std::string)package["name"];
    std::string cmaker_run_cmd = "cmaker run";

    cmdReturn ret = exec(cd_cmd + ";" + cmaker_init_cmd, thread_id);
    std::cout << "Checking package: " << (std::string)package["name"] << std::endl;
    // std::cout << report.size() << " packages failed " << (static_cast<float>(report.size()) / static_cast<float>(index)) * 100.0f << "%" << std::endl;
    if(checkOutForError(ret.out) || ret.status != 0){
      std::cout << "Failed to init package: " << (std::string)package["name"] << std::endl;
      item.error = "Failed to init";
      item.failed_on_add = false;
      item.failed_on_run = false;
      item.status_code = ret.status;
      item.full_cmd = cd_cmd + ";" + cmaker_init_cmd;
      item.full_error = ret.out;
      return item;
    }
    ret = exec(cd_cmd + ";" + cmaker_add_dep_cmd,thread_id);
    if(checkOutForError(ret.out) || ret.status != 0 ){
      std::cout << "Failed to add dep to package: " << (std::string)package["name"] << std::endl;
      item.error = "Failed to add dep";
      item.failed_on_add = true;
      item.failed_on_run = false;
      item.status_code = ret.status;
      item.full_cmd = cd_cmd + ";" + cmaker_add_dep_cmd;
      item.full_error = ret.out;
      return item;
    }
    ret = exec(cd_cmd + ";" + cmaker_run_cmd,thread_id);
    if(checkOutForError(ret.out) || ret.status != 0){
      std::cout << "Failed to run package: " << (std::string)package["name"] << std::endl;
      item.error = "Failed to run";
      item.failed_on_add = false;
      item.failed_on_run = true;
      item.status_code = ret.status;
      item.full_cmd = cd_cmd + ";" + cmaker_run_cmd;
      item.full_error = ret.out;
      return item;
    }
    return item;
}
int main(){
  using nlohmann::json;
  std::vector<std::future<reportItem>> results = std::vector<std::future<reportItem>>();
  int thread_count = 0;
  int max_threads = 150;
  std::ifstream index_file;
  json report = json::array();
  try{
    index_file = std::ifstream("./index.json");
  }catch(std::exception& e){
    std::cout << e.what() << std::endl;
    exit(1);
  }
  std::cout << "Opening file /index.json" << std::endl;
  std::string file_contents;
  for(std::string line; std::getline(index_file, line);){
    file_contents += line;
  }
  std::cout << "Attempting to parse json" << std::endl;
  json j;
  try{
    j = json::parse(file_contents);
  }catch(std::exception& e){
    std::cout << e.what() << std::endl;
    exit(1);
  }
  std::cout << "Parsed json" << std::endl;
  std::system("rm -rf /tmp/**/*");
  std::system("rm -rf /tmp/*");
  std::system("rm -rf /tmp/**/.*");
  int index = 0;
  for(auto& i : j){
    index++;
    if(thread_count < max_threads){
      thread_count++;

      results.push_back(std::async(std::launch::async,cmakerRunner,i,thread_count));
    }else{
      for(auto& result : results){
        reportItem item = result.get();
        json reportJson = json::object();
        reportJson["name"] = item.name;
        reportJson["version"] = item.version;
        reportJson["error"] = item.error;
        reportJson["failed_on_add"] = item.failed_on_add;
        reportJson["failed_on_run"] = item.failed_on_run;
        reportJson["full_cmd"] = item.full_cmd;
        reportJson["full_error"] = item.full_error;
        reportJson["status_code"] = item.status_code;
        report.push_back(reportJson);
        std::cout << "Package: " << item.name << " failed with error: " << item.error << std::endl;
        std::cout << report.size() << " packages failed " << (static_cast<float>(report.size()) / static_cast<float>(index)) * 100.0f << "%" << std::endl;
      }
      thread_count = 0;
    }
    //Gen package
  }
  return 0;
}
