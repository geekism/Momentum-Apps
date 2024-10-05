#!/usr/bin/env python3
import pathlib
import sys

import common


if __name__ == "__main__":
    common.check_workdir_state()

    if len(sys.argv) > 1:
        subtrees = sys.argv[1:]
    else:
        subtrees = list(common.REPO_ROOT.glob("**/.gitsubtree"))

    for subtree in subtrees:
        if not isinstance(subtree, pathlib.Path):
            if not subtree.endswith("/.gitsubtree"):
                subtree += "/.gitsubtree"
            subtree = common.REPO_ROOT / subtree
        path = subtree.parent.name
        print(f"\n\nUpdating {path}...")

        for remote in subtree.read_text().splitlines():
            if remote.startswith("#"):
                continue
            # TODO: add commit hash for subdir splits
            repo, branch, subdir = remote.split(" ")[:3]
            if subdir == "/":
                result, status = common.git(
                    "subtree",
                    "pull",
                    "-P",
                    path,
                    repo,
                    branch,
                    "-m",
                    f"Merge {path} from {repo}",
                    tee=True,
                )
                common.check_merge_result(path, repo, result, status)
            else:
                common.subdir_split_helper(path, repo, branch, subdir, "merge")

    # TODO: notifications
    # notify-send -a Git -i git "Subtree update finished" "Double check merge commits" &> /dev/null
