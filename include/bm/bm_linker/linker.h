/*
 * Hardik Soni (hardik.soni@inria.fr)
 *
 */

//! @file linker.h

#ifndef BM_BM_LINKER_H_
#define BM_BM_LINKER_H_

#include <vector>
#include <string>
#include <set>
#include <unordered_map> 

#include <bm/bm_sim/fields.h>
#include <bm/bm_sim/named_p4object.h>
#include <bm/bm_sim/phv_forward.h>
#include <bm/bm_sim/phv.h>
#include <bm/bm_sim/P4Objects.h>


#include <bm/bm_linker/link_headers.h>
#include <bm/bm_linker/link_parsers.h>
#include <bm/bm_linker/p4objects_linker_ext.h> 

namespace bm {


class Linker {
  
 public:

  explicit Linker();

  //! it gives pointer to new object
  std::shared_ptr<P4Objects> add_p4objects(const std::string& p4_name, 
                                            std::shared_ptr<P4Objects> objs);

  // Disabling copying, but allow moving
  Linker(const Linker &other) = delete;
  Linker &operator=(const Linker &) = delete;


 private:

  //! Create header type mappings
  ObjectLinkStateCode init_header_types_linking(
      const std::string& p4_name,
      const std::unordered_map<std::string, std::unique_ptr<HeaderType> >& 
        header_types_map);

  //! expands the all parser calls and returns single parser
  Parser* expand_all_sub_parser_calls(
      const std::unordered_map<std::string, std::unique_ptr<Parser> >& 
        p4objects_parser_map);

 private:
  // Linker keeps the shared pointer of running P4Objects
  // Probably, it should own the object. TBD
  std::shared_ptr<P4Objects> p4object_{nullptr};
  std::shared_ptr<P4ObjectsLinkerExt> p4objects_linker_ext_{nullptr};


  //! The instances of the types holds mapping between the running P4Objects 
  //! and all the added P4objects
  std::shared_ptr<HeaderTypeUIDTable> header_types_uid_table_{nullptr};
  std::shared_ptr<HeaderUIDTable> header_uid_table_{nullptr};

  LinkParsers parser_linker;

};


}  // namespace bm

#endif  // BM_BM_LINKER_H_
