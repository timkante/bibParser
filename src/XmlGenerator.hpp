#ifndef BTOOL_SRC_XMLGENERATOR_HPP_
#define BTOOL_SRC_XMLGENERATOR_HPP_

#include <AbstractGenerator.hpp>

/**
 * XML-Generator
 */
struct XmlGenerator : public AbstractGenerator {

  [[noreturn]] auto fail(const std::string &message, const std::string &type = "XML") -> void override;

  auto write() -> std::string override;

  explicit XmlGenerator(const std::vector<BibElement> &elements);
};

#endif
