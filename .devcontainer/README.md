# The template's development container

The container to work on the template in: every tool `tests/check.sh` needs, as it makes a
project from the template and runs all of that project's checks. VS Code offers two
configurations when running **Dev Containers: Reopen in Container**:

- **embedded-cpp-template (Linux)** for native Linux and remote environments. It mounts the
  host's SSH agent.
- **embedded-cpp-template (WSL2)** when VS Code runs in WSL2. It relies on VS Code's own
  SSH-agent forwarding and mounts nothing.

## The same image as every project

Both build `template/.devcontainer/Dockerfile`, the Dockerfile every project made from the
template gets; there is no second one. A change to it is a change to this container as well,
and `tests/check.sh` checks it as a project would. What the projects' containers are and carry
is in `template/.devcontainer/README.md.jinja`.

What differs lies in `shared/`: the names of the volumes, the Jinja extension for the files in
`template/`, and no `make setup` when the container is made, as the repository has no project
of its own. And the Ubuntu packages of the repository's own: the build argument `APT_PACKAGES`
names `template/.devcontainer/apt-packages.repo.txt` in place of a project's `apt-packages.txt`,
which the repository has only as a template. It holds gcc-avr and avr-libc, which
`tests/check.sh --no-arduino` and `--firmware` build with; the template's CI installs the same
list. A site-specific variant passes the argument as well. The first `tests/check.sh` installs the Arduino core into the volume
`embedded-cpp-template-arduino15`, where it stays; uv's cache, the Codex, Claude Code and gh logins
and the shell history have volumes of their own as well.

## Site-specific variants

Another directory beside `linux/` and `wsl2/` shows up as another entry in the same picker: the
place for what must not be committed, such as an internal base image through the `BASE_IMAGE`
build argument. `/.devcontainer/vector/` is listed in `.gitignore` for that.
