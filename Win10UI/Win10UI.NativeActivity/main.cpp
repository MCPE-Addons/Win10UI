#include <jni.h>
#include <sys/mman.h>

#include "log.h"
#include "substrate.h"
#include "dl_internal.h"
#include "hook/hook.h"

#include "OptionsScreen.h"
#include "OptionsGroup.h"
#include "AppPlatform_android23.h"
#include "ScreenChooser.h"

bool win10ui = true;
bool centeredGUI = true;

static const Options::Option WIN10UI("Enable", Options::OptionBoolean);
static const Options::Option CENTERED_GUI("Centered GUI", Options::OptionBoolean);

static const std::vector<int> GUI_SCALE_OFFSET_CUSTOM_VALUES = {-1, 0, 1, 2, 3, 4};

static void(*OptionsScreen$generateOptionScreens_real)(OptionsScreen *);
static void OptionsScreen$generateOptionScreens_hook(OptionsScreen *_this) {
	OptionsScreen$generateOptionScreens_real(_this);

	OptionsGroup *group = _this->optionsPanes[0]->createOptionsGroup("Windows 10 UI", true);
	group->createToggle(WIN10UI, *_this->client);
	group->createToggle(CENTERED_GUI, *_this->client);
}

static const std::vector<int> &(*Options$getValues_real)(Options *, const Options::Option*);
static const std::vector<int> &Options$getValues_hook(Options *_this, const Options::Option* option) {
	if (option == &Options::Option::GUI_SCALE)
		return GUI_SCALE_OFFSET_CUSTOM_VALUES;
	return Options$getValues_real(_this, option);
}

static bool(*Options$getBooleanValue_real)(Options *, const Options::Option*);
static bool Options$getBooleanValue_hook(Options *_this, const Options::Option* option) {
	if (option == &WIN10UI)
		return win10ui;
	else if (option == &CENTERED_GUI)
		return centeredGUI;
	return Options$getBooleanValue_real(_this, option);
}

static void(*Options$toggle_real)(Options *, const Options::Option*, int);
static void Options$toggle_hook(Options *_this, const Options::Option* option, int i) {
	if (option == &WIN10UI)
		win10ui = !win10ui;
	else if (option == &CENTERED_GUI)
		centeredGUI = !centeredGUI;
	else Options$toggle_real(_this, option, i);
}

static bool(*AppPlatform_custom$useMetadataDrivenScreens_real)(AppPlatform *);
static bool AppPlatform_custom$useMetadataDrivenScreens_hook(AppPlatform *_this) {
	return win10ui;
}

static bool(*AppPlatform_custom$useCenteredGUI_real)(AppPlatform *);
static bool AppPlatform_custom$useCenteredGUI_hook(AppPlatform *_this) {
	return centeredGUI;
}

static int(*AppPlatform_custom$getPlatformType_real)(AppPlatform *);
static int AppPlatform_custom$getPlatformType_hook(AppPlatform *_this) {
	return win10ui ? 0 : 1;
}

static std::string(*AppPlatform_custom$getEdition_real)(AppPlatform *);
static std::string AppPlatform_custom$getEdition_hook(AppPlatform *_this) {
	return win10ui ? "win10" : "pocket";
}

static UIScalingRules(*AppPlatform_custom$getPlatformUIScalingRules_real)(AppPlatform *);
static UIScalingRules AppPlatform_custom$getPlatformUIScalingRules_hook(AppPlatform *_this) {
	return win10ui ? UIScalingRules::Desktop : UIScalingRules::PocketEdition;
}

static UIScalingRules(*AppPlatform$getUIScalingRules_real)(AppPlatform *);
static UIScalingRules AppPlatform$getUIScalingRules_hook(AppPlatform *_this) {
	return win10ui ? UIScalingRules::Desktop : UIScalingRules::PocketEdition;
}

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
	soinfo2 *handle = (soinfo2 *)dlopen("libminecraftpe.so", RTLD_LAZY);
	MSHookFunction((void *)&OptionsScreen::generateOptionScreens, (void *)&OptionsScreen$generateOptionScreens_hook, (void **)&OptionsScreen$generateOptionScreens_real);
	MSHookFunction((void *)&Options::getValues, (void *)&Options$getValues_hook, (void **)&Options$getValues_real);
	MSHookFunction((void *)&Options::getBooleanValue, (void *)&Options$getBooleanValue_hook, (void **)&Options$getBooleanValue_real);
	MSHookFunction((void *)&Options::toggle, (void *)&Options$toggle_hook, (void **)&Options$toggle_real);
	
	void **AppPlatform_vtable = GetVtable("_ZTV11AppPlatform");
	VirtualHook(AppPlatform_vtable, 66, (void *)&AppPlatform_custom$useMetadataDrivenScreens_hook, (void **)&AppPlatform_custom$useMetadataDrivenScreens_real);
	VirtualHook(AppPlatform_vtable, 68, (void *)&AppPlatform_custom$useCenteredGUI_hook, (void **)&AppPlatform_custom$useCenteredGUI_real);
	VirtualHook(AppPlatform_vtable, 69, (void *)&AppPlatform_custom$getPlatformType_hook, (void **)&AppPlatform_custom$getPlatformType_real);
	VirtualHook(AppPlatform_vtable, 93, (void *)&AppPlatform_custom$getEdition_hook, (void **)&AppPlatform_custom$getEdition_real);
	VirtualHook(AppPlatform_vtable, 99, (void *)&AppPlatform_custom$getPlatformUIScalingRules_hook, (void **)&AppPlatform_custom$getPlatformUIScalingRules_real);

	void **AppPlatform_android_vtable = GetVtable("_ZTV19AppPlatform_android");
	VirtualHook(AppPlatform_android_vtable, 66, (void *)&AppPlatform_custom$useMetadataDrivenScreens_hook, (void **)&AppPlatform_custom$useMetadataDrivenScreens_real);
	VirtualHook(AppPlatform_android_vtable, 68, (void *)&AppPlatform_custom$useCenteredGUI_hook, (void **)&AppPlatform_custom$useCenteredGUI_real);
	VirtualHook(AppPlatform_android_vtable, 69, (void *)&AppPlatform_custom$getPlatformType_hook, (void **)&AppPlatform_custom$getPlatformType_real);
	VirtualHook(AppPlatform_android_vtable, 93, (void *)&AppPlatform_custom$getEdition_hook, (void **)&AppPlatform_custom$getEdition_real);
	VirtualHook(AppPlatform_android_vtable, 99, (void *)&AppPlatform_custom$getPlatformUIScalingRules_hook, (void **)&AppPlatform_custom$getPlatformUIScalingRules_real);

	void **AppPlatform_android23_vtable = GetVtable("_ZTV21AppPlatform_android23");
	VirtualHook(AppPlatform_android23_vtable, 66, (void *)&AppPlatform_custom$useMetadataDrivenScreens_hook, (void **)&AppPlatform_custom$useMetadataDrivenScreens_real);
	VirtualHook(AppPlatform_android23_vtable, 68, (void *)&AppPlatform_custom$useCenteredGUI_hook, (void **)&AppPlatform_custom$useCenteredGUI_real);
	VirtualHook(AppPlatform_android23_vtable, 69, (void *)&AppPlatform_custom$getPlatformType_hook, (void **)&AppPlatform_custom$getPlatformType_real);
	VirtualHook(AppPlatform_android23_vtable, 93, (void *)&AppPlatform_custom$getEdition_hook, (void **)&AppPlatform_custom$getEdition_real);
	VirtualHook(AppPlatform_android23_vtable, 99, (void *)&AppPlatform_custom$getPlatformUIScalingRules_hook, (void **)&AppPlatform_custom$getPlatformUIScalingRules_real);

	MSHookFunction((void *)&AppPlatform::getUIScalingRules, (void *)&AppPlatform$getUIScalingRules_hook, (void **)&AppPlatform$getUIScalingRules_real);

	return JNI_VERSION_1_2;
}