"""Exposes the ESPHome CLI as PlatformIO targets.

ESPHome with framework: esp-idf emits a plain CMake/ninja project, not a
PlatformIO one, so these targets shell out to the ESPHome CLI rather than
building anything through PlatformIO's own build system.
"""

import os
import shutil
import subprocess
import sys

Import("env")  # noqa: F821  (injected by PlatformIO)

PROJECT_DIR = env.subst("$PROJECT_DIR")  # noqa: F821
YAML = os.path.join(PROJECT_DIR, "esphome", "ventbridge.yaml")


def _esphome_bin():
    explicit = os.environ.get("ESPHOME_BIN")
    if explicit:
        return explicit

    venv = os.path.join(PROJECT_DIR, ".venv-esphome", "bin", "esphome")
    if os.path.isfile(venv):
        return venv

    found = shutil.which("esphome")
    if found:
        return found

    sys.stderr.write(
        "ESPHome not found. Install it with:\n"
        "  python3 -m venv .venv-esphome && .venv-esphome/bin/pip install esphome\n"
        "or set ESPHOME_BIN to an existing executable.\n"
    )
    env.Exit(1)  # noqa: F821


def _version():
    """Version string for esphome's `project.version`, taken from the git tag.

    On a GitHub release the tag is checked out, so this is the clean tag name;
    in between releases it carries the commit count and hash.
    """
    override = os.environ.get("ESPHOME_VERSION")
    if override:
        return override
    try:
        out = subprocess.run(
            ["git", "describe", "--tags", "--always", "--dirty"],
            cwd=PROJECT_DIR,
            capture_output=True,
            text=True,
            check=False,
        )
        if out.returncode == 0 and out.stdout.strip():
            return out.stdout.strip()
    except OSError:
        pass
    return "unknown"


def _run(*args):
    cmd = [_esphome_bin(), "-s", "version", _version(), *args, YAML]
    # ESPHome resolves relative paths in the YAML against its own location.
    result = subprocess.run(cmd, cwd=os.path.dirname(YAML), check=False)
    if result.returncode != 0:
        env.Exit(result.returncode)  # noqa: F821


def _device_args():
    device = os.environ.get("ESPHOME_DEVICE")
    return ["--device", device] if device else []


def esphome_compile(*_, **__):
    _run("compile")


def esphome_upload(*_, **__):
    # Without ESPHOME_DEVICE, ESPHome picks the port or OTA target itself.
    _run("run", "--no-logs", *_device_args())


def esphome_logs(*_, **__):
    _run("logs", *_device_args())


def esphome_clean(*_, **__):
    _run("clean")


for name, action, title, description in (
    ("esphome_compile", esphome_compile, "ESPHome: Compile", "Build the ESPHome firmware"),
    ("esphome_upload", esphome_upload, "ESPHome: Upload", "Build and flash (serial or OTA)"),
    ("esphome_logs", esphome_logs, "ESPHome: Logs", "Stream logs from the device"),
    ("esphome_clean", esphome_clean, "ESPHome: Clean", "Remove the ESPHome build tree"),
):
    env.AddCustomTarget(  # noqa: F821
        name=name,
        dependencies=None,
        actions=[action],
        title=title,
        description=description,
        always_build=True,
    )
