## Summary

<!-- What does this change and why? -->

## Related issues

<!-- Closes #... -->

## Testing

- [ ] `cmake --build build/native` succeeds
- [ ] `ctest --test-dir build/native --output-on-failure` passes (14 tests when EGL/GLES dev packages are installed)
- [ ] Manual check on device / emulator (describe below, or state "not performed")

## Checklist

- [ ] No signing keys, keystores, APKs, tokens or private uploads added
- [ ] Save-format compatibility and migration preserved
- [ ] Quality level still does not influence world generation or combat difficulty
- [ ] `AGENTS.md` / docs updated if behaviour or invariants changed
- [ ] `CHANGELOG.md` updated under "Unreleased"
