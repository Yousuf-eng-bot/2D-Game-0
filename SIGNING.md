# Signing continuity — read before building an APK in a new chat

The source ZIP deliberately contains **no private signing key**. Neither GitHub repository selection nor this source archive transfers the old workspace's private credentials.

The original key remains in the original workspace at `$HOME/.signing/ashen-prototype.jks`. The installed app and published prototypes use this public certificate fingerprint:

`6860d6fd6332594af5f30916fb67d9717088f63ebecb1c8d34476d27ae8f710b`

That fingerprint is public verification information, NOT the private key and cannot be used to sign updates.

## Updating the owner's existing game

1. Arrange a secure, private transfer/restore of the original keystore into the build environment, outside the repository. Do not paste its contents/passwords into source, chat logs or public issues. An appropriately configured private secret store is another option; no such integration is included or claimed here.
2. Set `SIGNING_KEY=/private/path/ashen-prototype.jks` if it is not in the default location.
3. Build with the same key and a monotonically increasing Android `versionCode` for future updates.
4. Validate with `tools/verify_apk.py`, which intentionally pins the original certificate. Update its expected app version when changing manifest version.

`tools/build_apk.sh` now fails early if the key is absent. It will **not silently generate another identity**. The prototype uses a development-key configuration; do not reuse that setup as an unreviewed production signing scheme.

## Disposable development build only

If you deliberately want a completely new, unrelated TEST signing identity:

```sh
ALLOW_NEW_SIGNING_KEY=1 SIGNING_KEY="$HOME/.signing/disposable-test.jks" bash tools/build_apk.sh
```

This is explicit opt-in. It cannot update the existing game signed with the original key. Use an isolated test device/profile. The normal verifier should reject its different certificate; do not weaken the original certificate check to disguise this difference.

**Never uninstall or clear the owner's game data to bypass an update-signature mismatch.** This game disables ordinary Android backup; changing signing identity carelessly can strand progress.
