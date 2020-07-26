#ifndef BIBPARSER_SRC_GENERATOREXCEPTION_HPP_
#define BIBPARSER_SRC_GENERATOREXCEPTION_HPP_

#include <boost/filesystem/path.hpp>
#include <stdexcept>
#include <string>

/**
 * A File-Writing exception
 */
struct GeneratorException  : std::runtime_error {
  std::string message; ///< the message to throw

  explicit GeneratorException(std::string message);

  [[nodiscard]] auto what() const noexcept -> const char * override;
};

#endif
