# 4-Person Team Workflow

1. Leader: GitHub → New repository → `cs631-sws-group4` → Public/Private as the class requires.
2. Leader in project folder: `git init && git add . && git commit -m "Initial four-module SWS starter" && git branch -M main && git remote add origin https://github.com/OWNER/cs631-sws-group4.git && git push -u origin main`.
3. Leader GitHub → Settings → Collaborators → Invite the other three accounts.
4. Each member clones, branches, edits **only** their component (`server.c`, `http.c`, `files.c`, `cgi.c`) and makes *real* improvements and tests.
5. Member `git push -u origin feature/member-N` → GitHub Pull Request → leader reviews/merges.
6. Everyone fetches merged main, runs `make clean && make`, and `python3 -m unittest discover -s tests -v`.
7. Integrate missing functionality according to `checklist.md`, collaboratively review docs, and retain real `git log`.
8. Run actual compatibility tests on the three required Unix platforms before submission.

## Work breakdown for group presentation
- **Person 1:** socket(), bind(), listen(), accept(), fork(), daemonization, select().
- **Person 2:** GET vs HEAD, HTTP/1.0 headers, If-Modified-Since, 304 responses.
- **Person 3:** root mapping, ~user path, directory index, safe paths, file MIME.
- **Person 4:** fork/exec CGI, environment variables, request routing and dynamic HTML.

**Do not present incomplete checklist items as completed.**
