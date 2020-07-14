#ifndef BIBPARSER_STYLEPROPERTIES_HPP
#define BIBPARSER_STYLEPROPERTIES_HPP

#include <string>
#include <vector>

/**
 * Stores information of a styles properties
 * @brief style-properties-container
 */
struct StyleProperties {
    std::string name = ""; ///< @property name of the style
    std::vector<std::string> requiredFields = {}; ///< @property all the required fields
    std::vector<std::string> optionalFields = {}; ///< @property all the optional fields

    /**
     * Default constructor.
     */
    StyleProperties() = default;

    StyleProperties(std::string name,
                    std::vector<std::string> requiredFields,
                    std::vector<std::string> optionalFields);

    auto operator==(const StyleProperties &other) const noexcept -> bool;
};

#endif
