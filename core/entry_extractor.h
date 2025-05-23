#ifndef ENTRY_EXTRACTOR_H
#define ENTRY_EXTRACTOR_H

#include <vector>
#include <string>
#include "entry.h"
#include "../config/config_handler.h"

class EntryExtractor {
public:
    virtual ~EntryExtractor() = default;

    // Pure virtual method to be implemented by subclasses
    virtual std::vector<Entry> extract_entries() = 0;
};

#endif // ENTRY_EXTRACTOR_H