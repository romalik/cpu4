#include <stdio.h>
#include <fstream>
#include <iostream>

#include <stdint.h>
#include <vector>

std::vector<std::string> ops = {
  "add",
  "sub",
  "neg",
  "shl",
  "NOP",
  "inc",
  "adc",
  "sbc",
  "not A",
  "and",
  "or",
  "xor",
  "zero",
  "NOP",
  "NOP",
  "NOP"
};



uint16_t hash(uint16_t input) {
  return (( (input >> 8) & 0xff ) ^ (input & 0xff));  
}

uint16_t build_addr(uint8_t A, uint8_t B, uint8_t cmd, uint8_t carry_in) {
  uint16_t result = (A                & 0b0000000001111) |  
                    ((B << 4)         & 0b0000011110000) |  
                    ((cmd << 8)       & 0b0111100000000) |  
                    ((carry_in << 12) & 0b1000000000000);
  return result;

}


uint8_t process(const std::vector<uint8_t> & image_first, const std::vector<uint8_t> & image_second, uint8_t A, uint8_t B, uint8_t cmd, uint8_t * FLAGS) {
  uint8_t A_low = A & 0xf;
  uint8_t A_high = (A & 0xf0) >> 4;
  
  uint8_t B_low = B & 0xf;
  uint8_t B_high = (B & 0xf0) >> 4;
  
  uint8_t carry_in_low = 0;

  uint8_t carry_in_high = 0;

  uint16_t addr_low = build_addr(A_low, B_low, cmd, carry_in_low);
  uint8_t result_low = image_first[addr_low];
  uint8_t F_low = result_low & 0b00001111;
  uint8_t zero_low = (result_low & 0b00010000) >> 4;
  uint8_t carry_low = (result_low & 0b00100000) >> 5;

  carry_in_high = carry_low;

  uint16_t addr_high = build_addr(A_high, B_high, cmd, carry_in_high);
  uint8_t result_high = image_second[addr_high];
  uint8_t F_high = result_high & 0b00001111;
  uint8_t zero_high = (result_high & 0b00010000) >> 4;
  uint8_t carry_high = (result_high & 0b00100000) >> 5;


  uint8_t F = F_low | (F_high << 4);
  *FLAGS = (zero_low & zero_high) | (carry_high << 1);
  return F;
}

void test(const std::vector<uint8_t> & image_first, const std::vector<uint8_t> & image_second) {

  auto do_test = [&](uint8_t A, uint8_t B, uint8_t cmd) -> bool {
    uint8_t expected = 0;
    switch(cmd) {
      //arithmetical
      case 0x0 : // add
      expected = A + B;
      break;
      case 0x1 : // sub
      expected = A - B;
      break;
      case 0x2 : // neg
      expected = -A;
      break;
      case 0x3 : // shl
      expected = A << 1;
      break;
      case 0x5 : // A++
      expected = A + 1;
      break;
      case 0x6 : // adc
      expected = A + B;
      break;
      case 0x7 : // sbc
      expected = A - B;
      break;
      //logical, no carry
      case 0x8 : // ~A
      expected = ~A;
      break;
      case 0x9 : // and
      expected = A & B;
      break;
      case 0xa : // or
      expected = A | B;
      break;
      case 0xb : // xor
      expected = A ^ B;
      break;
      case 0xc : // Zero
      expected = 0;
      break;
      default :
      expected = 0;
      break;
    }

    uint8_t result;
    uint8_t flags;

    result = process(image_first, image_second, A,B,cmd, &flags);

    if(result != expected) printf("Fail: A %d B %d op %d (%s) expected %d result %d flags %d\n", A, B, cmd, ops[cmd].c_str(), expected, result, flags);

    return result == expected;
  };

  size_t pass = 0;
  size_t fail = 0;
  for(int a = 0; a <= 0xff; a++) {
    for(int b = 0; b <= 0xff; b++) {
      for(int cmd = 0; cmd <= 0xf; cmd++) {
        if(do_test(a,b,cmd)) {
          pass++;
        } else {
          fail++;
        }
      }
    }
  }
  printf("Test:\npass: %zu\nfail: %zu\n", pass, fail);

}


uint8_t alu_value_for_input(uint16_t addr, int is_first_chip) {
  uint8_t A         =   (addr & 0b0000000001111) >> 0;
  uint8_t B         =   (addr & 0b0000011110000) >> 4;
  uint8_t cmd       =   (addr & 0b0111100000000) >> 8;
  uint8_t carry_in  =   (addr & 0b1000000000000) >> 12;


  uint8_t F = 0;
  uint8_t Z = 0;
  uint8_t C = 0;


  switch(cmd) {
    //arithmetical

    case 0x0 : // add
    F = A + B;
    if(!is_first_chip) {
      F += carry_in;
    }
    break;

    case 0x1 : // sub
    F = A - B;
    if(!is_first_chip) {
      F -= carry_in;
    }
    break;

    case 0x2 : // neg
    F = -A;
    if(!is_first_chip) {
      F -= carry_in;
    }
    break;

    case 0x3 : // shl
    F = A << 1;
    if(!is_first_chip) {
      F += carry_in;
    }

    break;

    case 0x5 : // A++
    if(is_first_chip) {
      F = A + 1;
    } else {
      F = A + carry_in;
    }
    break;

    case 0x6 : // ADC
      F = A + B + carry_in;
    break;

    case 0x7 : // SBC
      F = A - B - carry_in;
    break;


    //logical, no carry

    case 0x8 : // ~A
    F = ~A;
    break;

    case 0x9 : // and
    F = A & B;
    break;

    case 0xa : // or
    F = A | B;
    break;

    case 0xb : // xor
    F = A ^ B;
    break;

    case 0xc : // Zero
    F = 0;
    break;

    default :
    F = 0;
    break;

  }


  if(F & 0b11110000) {
    C = 1;
  }

  if((F & 0b00001111) == 0) {
    Z = 1;
  }

  uint8_t output =  (F        & 0b00001111) |
                    ((Z << 4) & 0b00010000) |
                    ((C << 5) & 0b00100000);
                    
  return output;
}


int main() {

  size_t image_size = 8*1024;


  std::vector<uint8_t> mem_first;
  std::vector<uint8_t> mem_second;
  mem_first.resize(image_size);
  mem_second.resize(image_size);

  for(uint16_t i = 0; i < image_size; i++) {
    //mem[i] = hash(i);
    mem_first[i] = alu_value_for_input(i, 1);
    mem_second[i] = alu_value_for_input(i, 0);
  }

  test(mem_first, mem_second);

  std::ofstream ofs_first("chip_low.bin");
  ofs_first.write((const char *)&mem_first[0], image_size);

  std::ofstream ofs_second("chip_high.bin");
  ofs_second.write((const char *)&mem_second[0], image_size);
}
