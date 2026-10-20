# make run      the game on the PC with the phone HUD (tools/scripts/ui_check.sh --mono),
#               a copy of SAVE in a scratch config, to try changes by hand
# make apk      the phone build: patched assemblies, Unity Android build, APK
# make install  build/oni-mobile.apk onto the phone through its installer
#
# The Unity build needs ~10.7 GB of free memory and must run alone (see docs/PLANO.md).

SAVE ?= $(HOME)/.config/unity3d/Klei/Oxygen Not Included/cloud_save_files/76561198338111860/Moonbase/Moonbase.sav
PLAY_DIR ?= build/ui-play

.PHONY: run apk install

run:
	tools/scripts/ui_check.sh --mono "$(SAVE)" $(PLAY_DIR) play

apk:
	tools/scripts/setup_player_project.sh
	tools/scripts/build_android.sh
	tools/scripts/make_apk.sh build/data-android

install:
	tools/scripts/install_phone.sh
