
/*
 * Hardik Soni (hardik.soni@inria.fr)
 *
 */

namespace cpp lswitch_runtime
namespace py lswitch_runtime

service LinkerSwitch {

  i32 p4_program_config_add(1:string program_name, 2:string config_str);
  i32 p4_program_config_delete(1:string program_name);

}
