/***************************************************************************
 * Copyright (c) 2025, Teddy Randby
 *
 * Distributed under the terms of the MIT license.
 *
 * The full license is in the file LICENSE, distributed with this software.
 ****************************************************************************/

#ifndef XEUS_GAB_INTERPRETER_HPP
#define XEUS_GAB_INTERPRETER_HPP

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif

#include <memory>
#include <string>

#include "nlohmann/json.hpp"

#include "xeus/xinterpreter.hpp"
#include "xeus_gab_config.hpp"

#include "../../../cgab/include/gab.h"

namespace nl = nlohmann;

namespace xeus_gab {
class XEUS_GAB_API interpreter : public xeus::xinterpreter {
private:
  struct gab_triple gab;
  gab_value env;
  a_gab_value* create_res;

public:
  interpreter();
  virtual ~interpreter() = default;

protected:
  void configure_impl() override;

  void execute_request_impl(send_reply_callback cb, int execution_counter,
                            const std::string &code,
                            xeus::execute_request_config config,
                            nl::json user_expressions) override;

  nl::json complete_request_impl(const std::string &code,
                                 int cursor_pos) override;

  nl::json inspect_request_impl(const std::string &code, int cursor_pos,
                                int detail_level) override;

  nl::json is_complete_request_impl(const std::string &code) override;

  nl::json kernel_info_request_impl() override;

  void shutdown_request_impl() override;
};
} // namespace xeus_gab

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#endif
