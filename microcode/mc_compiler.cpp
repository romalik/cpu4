#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <vector>
#include <map>
#include <unistd.h>
#include <string>
#include <bitset>
#include <sstream>
#include <deque>
#include <cmath>

std::string bin_to_str(uint64_t val, size_t width) {
  std::string full_addr = std::bitset<64>(val).to_string();
  return full_addr.substr(64 - width);
}

uint64_t str_to_bin(std::string str) {
  std::bitset<64> val;
  std::stringstream(str) >> val;
  return val.to_ulong();
}


std::string remove_comments(const std::string & str, const std::string & comment_chars = "#") {
    const auto comment_begin = str.find_first_of(comment_chars);
    if (comment_begin == std::string::npos)
        return str; // no content

    return str.substr(0, comment_begin);
}

std::string trim(const std::string& str,
                 const std::string& whitespace = " \t")
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

std::string reduce(const std::string& str,
                   const std::string& fill = " ",
                   const std::string& whitespace = " \t")
{
    // trim first
    auto result = trim(str, whitespace);

    // replace sub ranges
    auto beginSpace = result.find_first_of(whitespace);
    while (beginSpace != std::string::npos)
    {
        const auto endSpace = result.find_first_not_of(whitespace, beginSpace);
        const auto range = endSpace - beginSpace;

        result.replace(beginSpace, range, fill);

        const auto newStart = beginSpace + fill.length();
        beginSpace = result.find_first_of(whitespace, newStart);
    }

    return result;
}

size_t split(const std::string &txt, std::vector<std::string> &strs, char ch)
{
    size_t pos = txt.find( ch );
    size_t initialPos = 0;
    strs.clear();

    // Decompose statement
    while( pos != std::string::npos ) {
        std::string n_str = txt.substr( initialPos, pos - initialPos );
        if(!n_str.empty())
          strs.push_back(n_str);

        initialPos = pos + 1;

        pos = txt.find( ch, initialPos );
    }

    // Add the last one

    std::string n_str = txt.substr( initialPos, std::min( pos, txt.size() ) - initialPos + 1 );
    if(!n_str.empty()) 
      strs.push_back(n_str);

    return strs.size();
}

class ControlLine {
public:
  ControlLine() {}
  ControlLine(int pos, bool level, std::string _name): position(pos), active_level(level), name(_name) {}
  int position;
  bool active_level;
  std::string name;
};

std::map<std::string, ControlLine> control_lines;
std::map<std::string, int> config;
std::map<std::string, std::vector<std::string>> alias;

typedef std::pair<std::string, std::vector<std::string>> MInstrLine;
std::vector<MInstrLine> m_instr_lines;

std::vector<uint64_t> image;

void parse_error(int str_num) {
  printf("error on line %d\n", str_num);
  exit(1);
}

bool parse_microinstruction(const std::vector<std::string>& words, MInstrLine & m_instr) {
  std::string address_str;
  if(!config["address_width"]) {
    printf("Address width not set\n");
    return false;
  }
  size_t cnt = 0;
  while(address_str.length() < config["address_width"]) {
    address_str += words[cnt];
    cnt++;
  }

  if(address_str.length() != config["address_width"]) {
    printf("Address length mismatch\n");
    return false;
  }

  m_instr.first = address_str;
  for(size_t i = cnt; i<words.size(); i++) {
    std::string id = words[i];
    if(alias.find(id) != alias.end()) {
      m_instr.second.insert(m_instr.second.end(), alias[id].begin(), alias[id].end());
    } else if(control_lines.find(words[i]) != control_lines.end()) {
      m_instr.second.push_back(words[i]);
    } else {
      printf("Unknown control line %s\n", words[i].c_str());
      return false;
    }
  }
  return true;

}

void parse_file(const std::string & path) {
  std::ifstream ifs(path);

  std::string current_section;

  std::string line;
  int line_cnt = 1;
  while(std::getline(ifs, line)) {
    line = remove_comments(line);
    line = reduce(line);

    std::vector<std::string> words;
    split(line, words, ' ');
    if(!words.empty()) {
      if(words[0] == "") continue;
/*
      printf("words size: %zu\n", words.size());
      int cnt = 0;
      for(const auto & w : words) {
        printf("%d : \"%s\"\n", cnt, w.c_str());
        cnt++;
      }
*/
      if(words[0] == "section") {
        if(current_section != "") {
          parse_error(line_cnt);
        }
        if(words.size() < 2) {
          parse_error(line_cnt);
        }
        current_section = words[1];


      } else if(words[0] == "end") {
        if(current_section == "") {
          parse_error(line_cnt);
        }
        current_section = "";


      } else if(current_section == "config") {
        if(words.size() < 2) {
          parse_error(line_cnt);
        }
        config[words[0]] = std::stoi(words[1]);

      } else if(current_section == "control_word") {
        if(words.size() < 3) {
          parse_error(line_cnt);
        }

        control_lines[words[2]] = ControlLine(std::stoi(words[0]), ((words[1] == "H")?true:false), words[2]);

      } else if(current_section == "alias") {
        if(words.size() < 2) {
          parse_error(line_cnt);
        }

        alias[words[0]] = std::vector<std::string>(words.begin()+1, words.end());

      } else if(current_section == "code") {
        MInstrLine m_line;
        if(!parse_microinstruction(words, m_line)) {
          parse_error(line_cnt);          
        }        

        m_instr_lines.push_back(m_line);

      }


    }
    line_cnt++;
  }
}




void process_dashes() {
  for(int i = 0; i<m_instr_lines.size(); i++) {
    MInstrLine & m_line = m_instr_lines[i];
    for(int pos = 0; pos < m_line.first.length(); pos++) {
      if(m_line.first[pos] == '-') {
        if(i == 0) {
          printf("First address cannot contain '-'\n");
          exit(1);
        }
        char prev_value = m_instr_lines[i-1].first[pos];

        if(prev_value == '*') {
          printf("Dash after asterisk for address %s\n", m_line.first.c_str());
          exit(1);
        }

        m_line.first[pos] = prev_value;
      }
    }
  }
}

uint64_t get_default_word() {
  uint64_t word = 0;

  for(const auto & cl : control_lines) {
    if(!cl.second.active_level) {
      word |= (1ULL << cl.second.position);
    }
  }
  return word;
}

std::vector<uint64_t> get_address_vector(const std::string & address_mask) {

  std::deque<std::string> not_processed;  

  not_processed.push_back(address_mask);

  std::vector<uint64_t> result;

  while(!not_processed.empty()) {
    auto adr = not_processed.front();
    not_processed.pop_front();
    auto asterisk_position = adr.find_first_of("*");
    if(asterisk_position != std::string::npos) {
      std::string new_address_1 = adr;
      std::string new_address_2 = adr;
      new_address_1[asterisk_position] = '0';
      new_address_2[asterisk_position] = '1';
      not_processed.push_back(new_address_1);
      not_processed.push_back(new_address_2);
    } else {
      result.push_back(str_to_bin(adr));
    }

  }
  return result;

}


void compile() {
  uint64_t default_word = get_default_word();

  size_t image_size = (1 << config["address_width"]);
  image.clear();
  image.resize(image_size, default_word);

  std::vector<int> visited_mask(image_size, 0);
  

  for(const auto & instr : m_instr_lines) {
    auto adr_vec = get_address_vector(instr.first);
    uint64_t word = default_word;
    for(const auto & cl : instr.second) {
      if(control_lines.find(cl) == control_lines.end()) {
        printf("Unknown control line %s\n", cl.c_str());
        exit(1);
      }
      ControlLine & c_line = control_lines[cl];

      if(c_line.active_level) {
        word |= (1ULL << c_line.position);
      } else {
        word &= ~(1ULL << c_line.position);
      }
    }

    for(const auto & adr : adr_vec) {
      if(visited_mask[adr]) {
        printf("Double write at addr %s\n", bin_to_str(adr, config["address_width"]).c_str());
        exit(1);
      }
      image[adr] = word;
      visited_mask[adr] = 1;
    }
  }
}


std::vector<std::vector<uint8_t>> split_8bit() {
  std::vector<std::vector<uint8_t>> result;
  int width = config["control_width"];
  int n_chips = static_cast<int>(std::ceil(static_cast<float>(width) / 8.0f));

  result.resize(n_chips, std::vector<uint8_t>(image.size(), 0));

  for(size_t i = 0; i<image.size(); i++) {
    for(int chunk = 0; chunk < n_chips; chunk++) {
      uint64_t word_part = ((image[i] & (0xffULL << (chunk*8))) >> (chunk*8));
      result[chunk][i] = word_part & 0xff;
    }
  }

  return result;
}


int main(int argc, char ** argv) {
  if(argc < 2) {
    printf("Usage %s microcode.txt\n", argv[0]);
    exit(1);
  }

  parse_file(argv[1]);

  process_dashes();

/*
  for(auto cl : control_lines) {
    printf("cl: %d %s %s\n", cl.second.position, cl.second.active_level?"1":"0", cl.second.name.c_str());
  }

  for(auto cfg: config) {
    printf("cfg: %s %d\n", cfg.first.c_str(), cfg.second);
  }

  for(auto al: alias) {
    printf("alias %s = ", al.first.c_str());
    for(auto c : al.second) {
      printf("%s ", c.c_str());
    }
    printf("\n");
  }

  printf("code:\n");
  for(auto ml: m_instr_lines) {
    printf("%s : ", ml.first.c_str());
    for(auto c : ml.second) {
      printf("%s ", c.c_str());
    }
    printf("\n");
  }
*/
  compile();

  auto chip_images = split_8bit();

  for(size_t n_chip = 0; n_chip < chip_images.size(); n_chip++) {
    std::string filename = "microcode_" + std::to_string(n_chip) + ".bin";
    std::ofstream ofs(filename, std::ios::out | std::ios::binary);
    ofs.write((char *)&chip_images[n_chip][0], chip_images[n_chip].size());
  }

} 