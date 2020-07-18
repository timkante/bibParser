#include <boost/filesystem/path.hpp>
#include <GlobalParserState.hpp>
#include <ParserException.hpp>
#include <spdlog/spdlog.h>
#include <Parser.hpp>
#include <algorithm>
#include <numeric>

using namespace std::literals::string_literals;

/**
 * Constructor.
 * @param ruleFilePath filepath to the translation-table
 * @param targetStyle the style name to filter parse-results for
 */
Parser::Parser(const boost::filesystem::path &ruleFilePath, std::vector<std::string> targetStyles) noexcept
    : targetStyles(std::move(targetStyles)), translationTable(TranslationTable(ruleFilePath)) {}

/**
 * Constructor.
 * @param ruleFileContents contents of the translation-table
 * @param targetStyle the style name to filter parse-results for
 */
Parser::Parser(std::stringstream ruleFileContents, std::vector<std::string> targetStyles) noexcept
    : targetStyles(std::move(targetStyles)), translationTable(TranslationTable(std::move(ruleFileContents))) {}

/**
 * Generate bib-elements and Filter them for a Style
 * @param inputPath filepath to a directory of bib-Files or a single bib-File
 * @param sorting the style name to sort parsing-results for
 * @return collection of parsed bib-elements
 */
auto Parser::generate(
    const boost::filesystem::path &inputPath,
    const std::optional<std::string> &sorting
) const noexcept -> std::vector<BibElement> {
  if (!boost::filesystem::exists(inputPath)) {
    spdlog::critical("No such file or directory. [input={}]", inputPath.string());
    return {};
  } else if (boost::filesystem::is_directory(inputPath)) {
    std::vector<BibElement> collector{};
    for (const auto &file : boost::filesystem::directory_iterator(inputPath)) {
      spdlog::info("Parsing File: {} ...", file.path().string());
      std::ifstream inFile{file.path().string()};
      inFile >> std::noskipws;
      std::string inContent{std::istream_iterator<char>{inFile}, std::istream_iterator<char>{}};
      for (const auto &element : generate(std::string_view(inContent), sorting, inputPath.string())) {
        collector.push_back(element);
      }
    }
    if (sorting) sortElements(collector, sorting.value());
    return collector;
  } else if (boost::filesystem::is_regular_file(inputPath)) {
    spdlog::info("Parsing File: {} ...", inputPath.string());
    std::ifstream inFile{inputPath.string()};
    inFile >> std::noskipws;
    std::string inContent{std::istream_iterator<char>{inFile}, std::istream_iterator<char>{}};
    auto elements = generate(std::string_view(inContent), sorting, inputPath.string());
    if (sorting) sortElements(elements, sorting.value());
    return elements;
  } else {
    spdlog::critical("Unexpected file-descriptor. [input={}]", inputPath.string());
    return {};
  }
}

/**
 * Generate bib-elements and Filter them for a Style
 * @param inputFileContent content of a input-file
 * @param sorting the style name to sort parsing-results for
 * @param filename name or path of the parsed file (for logging errors)
 * @return collection of parsed, sorted and filtered bib-elements
 */
auto Parser::generate(
    std::string_view inputFileContent,
    const std::optional<std::string> &sorting,
    const std::string &filename
) const noexcept -> std::vector<BibElement> {
  const std::vector<StyleProperties> targetStructures = translationTable.stylePropertiesOf(targetStyles);
  if (targetStructures.empty()) {
    return {};
  } else {
    const auto parsedElements = Parser::elementsOf(inputFileContent, filename);
    std::vector<BibElement> filteredElements{};
    std::copy_if(
        std::cbegin(parsedElements),
        std::cend(parsedElements),
        std::back_inserter(filteredElements),
        [&](const BibElement &element) {
          return std::find_if(
              std::cbegin(targetStructures),
              std::cend(targetStructures),
              [&](const StyleProperties &prop) {
                return element.isCompliantTo(prop);
              }
          ) != std::cend(targetStructures);
        }
    );
    if (sorting) sortElements(filteredElements, sorting.value());
    return filteredElements;
  }
}

/**
 * Parse Bib-Elements from a source-file
 * @param input the contents of the source-file
 * @param filename name or path of the parsed file (for logging errors)
 * @return collection of parsed bib-elements
 */
auto Parser::elementsOf(
    std::string_view input,
    const std::string &filename
) noexcept -> std::vector<BibElement> {
  ParserContext context(filename);
  std::vector<BibElement> result;
  try {
    delete std::accumulate<std::string_view::iterator, ParserState *>(
        std::cbegin(input),
        std::cend(input),
        new GlobalParserState{context, result},
        [&context](ParserState *const acc, const char it) -> ParserState * {
          ++context.column;
          if (it == '\n') {
            ++context.line;
            context.column = 0;
            return acc;
          }
          return acc->handleCharacter(it);
        });
    return result;
  } catch (ParserException &e) {
    spdlog::critical("Parsing failed: {}", e.what());
    delete e.state;
    return {};
  }
}

/**
 * Sorts a vector of bibElements
 * @param elements[out] the vector of bibElements
 * @param sorting the attribute to sort for
 */
auto Parser::sortElements(std::vector<BibElement> &elements, const std::string &sorting) noexcept -> void {
  std::sort(
      std::begin(elements),
      std::end(elements),
      [&sorting](const BibElement &l, const BibElement &r) {
        return l.findAttribute(sorting).value_or<Field>({""s, ""s}).value
            < r.findAttribute(sorting).value_or<Field>({""s, ""s}).value;
      }
  );
}

auto Parser::generate(
    const std::vector<boost::filesystem::path> &inputPaths,
    const std::optional<std::string> &sorting
) const noexcept -> std::vector<BibElement> {
  std::vector<BibElement> parsedElements;
  std::for_each(
      std::cbegin(inputPaths),
      std::cend(inputPaths),
      [&](const boost::filesystem::path &p) {
        const auto elements = generate(p, sorting);
        parsedElements.insert(std::end(parsedElements), std::cbegin(elements), std::cend(elements));
      }
  );
  if (sorting) sortElements(parsedElements, sorting.value());
  return parsedElements;
}
