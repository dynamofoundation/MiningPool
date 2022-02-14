#include "Global.h"

Global::Global() {
	settings = new Settings();
	settings->readSettings();
}