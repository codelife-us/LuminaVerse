# Plans to consider 

## Future Development

### Additional Gospel Tracts

The program is designed to support multiple gospel presentation tracts. Currently, only "The Romans Road" is implemented, but the architecture allows for easy addition of new tracts.

### Tract Sources

**The Romans Road** - Originally sourced from https://mwtb.org/

### Possible Technical Improvements

- May add more Bible translations
- Support for custom tract creation via configuration files
- Export functionality (PDF, HTML, plain text)
- Interactive mode for step-by-step presentation
- Localization support for multiple languages

### Distribution

- Package for different operating systems
- Web-based version
- Mobile app versions (iOS/Android)

## Contributing

To add a new tract, create a new `vector<Section>` with the tract content and register it in the `availableTracts` map in `gospel.cpp`.

Example structure:
```cpp
vector<Section> newTractSections = {
    {"Title", "Content...", "Reference", "kjv_text", "bsb_text"},
    // ... more sections
};

availableTracts["New Tract Name"] = {"New Tract Name", newTractSections};
```