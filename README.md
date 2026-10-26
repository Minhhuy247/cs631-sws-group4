# CS631 SWS — Group of Four

A teaching **starter implementation in C** of the CS631 `sws(1)` assignment. Four owners, one C module per member, integrated via `sws.h` and `Makefile`.

## Quick start on Ubuntu

```sh
sudo apt update
sudo apt install build-essential curl python3
make
./sws -d -i 127.0.0.1 -p 8080 -c cgi-bin www
```

Open http://127.0.0.1:8080 in Firefox. Use Ctrl+C to stop debug mode. In a second terminal run `python3 -m unittest discover -s tests -v` (tests launch their own isolated port).

## Command line

`./sws [-dh] [-c dir] [-i address] [-l file] [-p port] dir`

- `-d`: foreground, sequential clients, common-format log to stdout.
- No `-d`: daemon, fork a child per connection.
- `-h`: usage, `-i`: bind IP, `-p`: listening port (default 8080).
- `-c`: CGI directory (absolute or current-directory relative).
- `-l`: append common-format request logs to file (normally disabled).
- `dir`: document root.

**Daemon example:** `./sws -i 127.0.0.1 -p 8080 -l requests.log -c cgi-bin www`. Stop with `pkill -f '/sws '` only if you have checked the process first (`pgrep -af sws`); use Ctrl+C for development.

## The four-member split

| Owner | File | Duties | Acceptance tests |
|---|---|---|---|
| Member 1 | `server.c` | CLI options, bind IPv4/IPv6, fork, daemon, logs | start/stop, `-d`, `-h`, simultaneous connections |
| Member 2 | `http.c` | HTTP request syntax, GET/HEAD, HTTP/1.0 reply, timestamps, status codes | request methods, headers, 304 |
| Member 3 | `files.c` | docroot, safe path resolution, static content, directory listing, ~user translation | GET, index, directory, traversal |
| Member 4 | `cgi.c` | CGI process environment, pipe capture, result | CGI success, fail, HEAD |

Each member edits **their assigned `.c` only** on their own Git branch, writes tests for that module, and contributes commits. Shared changes (`sws.h`, Makefile) require group discussion. The provided baseline is for learning; distribute real debugging, test and improvement work equally. Record actual participants and their contributions; do not invent git commits.

## Test commands and evidence

```sh
make clean && make
python3 -m unittest discover -s tests -v
curl -i http://127.0.0.1:8080/docs/hello.txt
curl -I http://127.0.0.1:8080/docs/hello.txt
curl -i -H 'If-Modified-Since: Wed, 01 Jan 2100 00:00:00 GMT' http://127.0.0.1:8080/docs/hello.txt
curl -i 'http://127.0.0.1:8080/cgi-bin/hello.sh?name=student'
```

## IMPORTANT: remaining work before claiming compliance

This is a **working prototype, not a verified full RFC1945/RFC3875 or cross-platform submission**. Known gaps:

1. MIME detection currently uses a small filename-extension table, not `magic(5)` as the official manual requires.
2. CGI is a basic GET/HEAD stdout-to-response bridge. It lacks robust CGI Status/Location handling, RFC3875 full environment, execution timeout, strict output cap/error behavior.
3. Several error paths are simplified, including error responses from CGI, malformed request line, and some status codes. The request log currently logs `0` as byte count for successful static responses; fix accounting.
4. Directory listing is alphabetical but needs HTML escaping, URL encoding, safe link generation, and better large-directory behavior.
5. `~username` resolves using system passwd home + `/sws`, requiring that directory to be created and readable for the actual user.
6. No full NetBSD 11 / OmniOS compile or runtime testing has been performed. Portability and compatibility must be validated on the required servers. Also review `realpath`, `timegm`, signals, `fd_set`, C11 flags on each platform.
7. Add more HTTP edge-case tests, concurrency/daemon tests, timeouts, and security hardening before exposure beyond localhost.

## Git: four people, four branches

The group leader creates a GitHub repository, uploads this *starter* on `main`, invites all three collaborators, and protects `main` if available.

```sh
# Each member clones the repository URL supplied by the leader:
git clone https://github.com/OWNER/REPO.git
cd REPO
# Change N to your own member number: 1,2,3,4
git switch -c feature/member-N
# Edit only your assigned file, implement improvements, test.
git add server.c  # Member 1; other members use their assigned file
git commit -m 'Improve member N component'
git push -u origin feature/member-N
```

On GitHub each member opens a Pull Request to `main`. Leader reviews and merges PRs. Then everyone runs `git pull origin main && make clean && make && python3 -m unittest discover -s tests -v`.

### Final archive (after real team names are known)

```sh
cd ..
mv REPO username1-username2-username3-username4
# Remove binaries, secrets and old test output before delivery.
tar -cf username1-username2-username3-username4.tar username1-username2-username3-username4/
```

The README in your **final submission** must record real work sharing, Git evidence, supported platforms, known limitations, and commands used. Follow instructor submission rules.

## Requirements mapping

Source: `https://stevens.netmeister.org/631/sws.1.pdf` and provided CS631 project statement. See `checklist.md` for a progress checklist.
