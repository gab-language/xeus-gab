/***************************************************************************
 * Copyright (c) 2025, Teddy Randby
 *
 * Distributed under the terms of the MIT license.
 *
 * The full license is in the file LICENSE, distributed with this software.
 ****************************************************************************/

#include <iostream>
#include <ostream>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

#include "xeus/xhelper.hpp"
#include "xeus/xinput.hpp"
#include "xeus/xinterpreter.hpp"

#include "xeus-gab/xinterpreter.hpp"

namespace nl = nlohmann;

namespace xeus_gab {

static const char *default_modules[] = {
    "Strings", "Binaries", "Messages", "Numbers",  "Blocks",
    "Records", "Shapes",   "Fibers",   "Channels", "__core",
    "Ranges",  "Streams",  "IO",
};
static const char *default_module_values[] = {
    tGAB_STRING, tGAB_BINARY, tGAB_MESSAGE, tGAB_NUMBER,  tGAB_BLOCK,
    tGAB_RECORD, tGAB_SHAPE,  tGAB_FIBER,   tGAB_CHANNEL, "__core",
    "Ranges",    "Streams",   "IO",
};
static const size_t ndefault_modules = LEN_CARRAY(default_modules);

interpreter::interpreter() {
  xeus::register_interpreter(this);

  union gab_value_pair result = gab_create(
      {
          .flags = 0,
          .jobs = 8,
          .len = ndefault_modules,
          .modules = default_modules,
      },
      &gab);

  assert(result.status == gab_cvalid);
  assert(result.aresult->len > 0);

  create_res = result.aresult;
  env = gab_cinvalid;
}

bool check_res(struct gab_triple gab, union gab_value_pair res,
               interpreter::send_reply_callback cb) {
  if (res.status != gab_cvalid) {
    gab_value *err = gab_egerrs(gab.eg);
    std::string error;

    for (gab_value *thiserr = err; *thiserr != gab_nil; thiserr++) {
      assert(gab_valkind(*thiserr) == kGAB_RECORD);

      if (*thiserr == res.vresult)
        continue;

      const char *errstr = gab_errtocs(gab, *thiserr);
      assert(errstr != nullptr);

      error += errstr;
    };

    free(err);

    const char *errstr = gab_errtocs(gab, res.vresult);
    assert(errstr != nullptr);

    error += errstr;

    cb(xeus::create_error_reply(error));

    return true;
  }

  return false;
}

bool check_vresult(struct gab_triple gab, union gab_value_pair res,
                   interpreter::send_reply_callback cb) {
  return check_res(gab, res, cb);
}

bool check_aresult(struct gab_triple gab, union gab_value_pair res,
                   interpreter::send_reply_callback cb) {
  if (check_res(gab, res, cb))
    return true;

  if (res.aresult->data[0] != gab_ok) {
    const char *errstr = gab_errtocs(gab, res.aresult->data[1]);
    assert(errstr != nullptr);

    cb(xeus::create_error_reply(errstr));

    return true;
  }

  return false;
}

void interpreter::execute_request_impl(
    send_reply_callback cb,  // Callback to send the result
    int execution_counter,   // Typically the cell number
    const std::string &code, // Code to execute
    xeus::execute_request_config config, nl::json /*user_expressions*/) {

  // If we *do* have an environment, just use that.
  size_t len = env == gab_cinvalid ? 0 : (gab_reclen(env) - 1);
  std::vector<const char *> keys = {};
  std::vector<gab_value> keyvals = {};
  std::vector<gab_value> vals = {};

  for (size_t i = 0; i < len; i++) {
    size_t index = i + 1;
    keyvals.push_back(gab_ukrecat(env, index));
    keys.push_back(gab_strdata(&keyvals[i]));
    vals.push_back(gab_uvrecat(env, index));
  }

  for (size_t i = 0; i < ndefault_modules; i++) {
    if (env != gab_cinvalid &&
        gab_rechas(env, gab_string(gab, default_modules[i])))
      continue;

    keys.push_back(default_modules[i]);
    vals.push_back(gab_message(gab, default_module_values[i]));
  }

  union gab_value_pair fiber =
      gab_aexec(gab, (struct gab_exec_argt){
                         .name = std::to_string(execution_counter).c_str(),
                         .source = code.c_str(),
                         .len = keys.size(),
                         .sargv = keys.data(),
                         .argv = vals.data(),
                         .flags = 0,
                     });

  if (check_vresult(gab, fiber, cb))
    return;

  union gab_value_pair result = gab_fibawait(gab, fiber.vresult);

  /* Setup env regardless of run failing/succeeding */
  gab_value new_env = gab_fibawaite(gab, fiber.vresult);

  // If either environment is bad, use new.
  // In next iteration, we will use default values.
  if (env == gab_cinvalid || new_env == gab_cinvalid)
    env = new_env;
  // Both of our environments are valid, and we can concatenate them!
  else
    env = gab_reccat(gab, env, new_env);

  assert(env != gab_cinvalid);

  if (check_aresult(gab, result, cb))
    return;

  // Stringify these into a list please
  gab_value rec = gab_list(gab, result.aresult->len, result.aresult->data);
  gab_value str = gab_valintos(gab, rec);

  // Use this method for publishing the execution result to the client,
  // this method takes the ``execution_counter`` as first argument,
  // the data to publish (mime type data) as second argument and metadata
  // as third argument.
  // Replace "Hello World !!" by what you want to be displayed under the
  // execution cell
  nl::json pub_data;
  pub_data["text/plain"] = gab_strdata(&str);

  if (!config.silent)
    publish_execution_result(execution_counter, pub_data, nl::json::object());

  return cb(xeus::create_successful_reply());
}

void interpreter::configure_impl() {
  // `configure_impl` allows you to perform some operations
  // after the custom_interpreter creation and before executing any request.
  // This is optional, but can be useful;
  // you can for example initialize an engine here or redirect output.
}

nl::json interpreter::is_complete_request_impl(const std::string &code) {
  // Insert code here to validate the ``code``
  // and use `create_is_complete_reply` with the corresponding status
  // "unknown", "incomplete", "invalid", "complete"
  return xeus::create_is_complete_reply("complete");
}

nl::json interpreter::complete_request_impl(const std::string &code,
                                            int cursor_pos) {

  // No completion result
  return xeus::create_complete_reply(nl::json::array(), /*matches*/
                                     cursor_pos,        /*cursor_start*/
                                     cursor_pos         /*cursor_end*/
  );
}

nl::json interpreter::inspect_request_impl(const std::string & /*code*/,
                                           int /*cursor_pos*/,
                                           int /*detail_level*/) {

  return xeus::create_inspect_reply();
}

void interpreter::shutdown_request_impl() { gab_destroy(gab); }

nl::json interpreter::kernel_info_request_impl() {

  const std::string protocol_version = "5.3";
  const std::string implementation = "xgab";
  const std::string implementation_version = XEUS_GAB_VERSION;
  const std::string language_name = "gab";
  const std::string language_version = GAB_VERSION_TAG;
  const std::string language_mimetype = "text/x-gab";
  ;
  const std::string language_file_extension = "gab";
  ;
  const std::string language_pygments_lexer = "";
  const std::string language_codemirror_mode = "";
  const std::string language_nbconvert_exporter = "";
  const std::string banner = "xgab";
  const bool debugger = false;

  const nl::json help_links = nl::json::array();

  return xeus::create_info_reply(
      protocol_version, implementation, implementation_version, language_name,
      language_version, language_mimetype, language_file_extension,
      language_pygments_lexer, language_codemirror_mode,
      language_nbconvert_exporter, banner, debugger, help_links);
}

} // namespace xeus_gab
