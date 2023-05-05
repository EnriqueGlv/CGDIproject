#ifndef SETTINGS_H
#define SETTINGS_H

class Settings {
    public:
        bool doBackwardSearch;
        bool showEnergy;
        int seamsToRemove;
        bool isEqual(const Settings &other) {
            return (other.doBackwardSearch == doBackwardSearch &&
                    other.showEnergy == showEnergy &&
                    other.seamsToRemove == seamsToRemove);
        };
};

#endif // SETTINGS
