#include <TranslationTable.hpp>
#include <FieldConstraint.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/optional/optional.hpp>
#include <optional>
#include <iterator>

TranslationTable::TranslationTable(std::stringstream file) {
    boost::property_tree::read_json(file, contents);
    styleProps = std::vector<StyleProperties>();
    commonProps = parseCommonProperties();
}

TranslationTable::TranslationTable(const boost::filesystem::path &path) {
    boost::property_tree::read_json(path.string(), contents);
    styleProps = std::vector<StyleProperties>();
    commonProps = parseCommonProperties();
}

auto TranslationTable::printAll(std::ostream &out) const -> void {
    boost::property_tree::json_parser::write_json(out, contents);
}

auto TranslationTable::parseCommonProperties() const -> StyleProperties {
    const auto parseConstraintNode = [](const boost::property_tree::ptree &node) {
        assert(std::all_of(std::cbegin(node),
                           std::cend(node),
                           [](const boost::property_tree::ptree::value_type &field) {
                               return field.first.empty();
                           }));
        std::vector<FieldConstraint> fields;
        std::for_each(std::cbegin(node),
                      std::cend(node),
                      [&fields](const boost::property_tree::ptree::value_type &field) {
                          const boost::optional<const boost::property_tree::ptree &> title =
                                  field.second.get_child_optional("title");
                          const boost::optional<const boost::property_tree::ptree &> format =
                                  field.second.get_child_optional("format");
                          if (title && format && !title.value().data().empty() && !format.value().data().empty()) {
                              fields.emplace_back(title.value().data(), std::regex{format.value().data()});
                          }
                      });
        return fields;
    };

    const boost::optional<const boost::property_tree::ptree &> requiredFieldsNode =
            contents.get_child_optional("commonRequiredFields");
    const boost::optional<const boost::property_tree::ptree &> optionalFieldsNode =
            contents.get_child_optional("commonOptionalFields");

    return StyleProperties("_common",
                           requiredFieldsNode ? parseConstraintNode(requiredFieldsNode.value())
                                              : std::vector<FieldConstraint>{},
                           optionalFieldsNode ? parseConstraintNode(optionalFieldsNode.value())
                                              : std::vector<FieldConstraint>{});
}
