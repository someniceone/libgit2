/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */
#ifndef INCLUDE_merge_h__
#define INCLUDE_merge_h__

#include "common.h"

#include "vector.h"
#include "commit_list.h"
#include "pool.h"
#include "iterator.h"

#include "git2/types.h"
#include "git2/merge.h"
#include "git2/sys/merge.h"

#define GIT_MERGE_MSG_FILE		"MERGE_MSG"
#define GIT_MERGE_MODE_FILE		"MERGE_MODE"
#define GIT_MERGE_FILE_MODE		0666

#define GIT_MERGE_DEFAULT_RENAME_THRESHOLD	50
#define GIT_MERGE_DEFAULT_TARGET_LIMIT		1000


/** Internal merge flags. */
enum {
	/** The merge is for a virtual base in a recursive merge. */
	GIT_MERGE__VIRTUAL_BASE = (1 << 31),
};

enum {
	/** Accept the conflict file, staging it as the merge result. */
	GIT_MERGE_FILE_FAVOR__CONFLICTED = 4,
};

typedef struct {
	git_repository *repo;
	git_pool pool;

	/* Vector of git_index_entry that represent the merged items that
	 * have been staged, either because only one side changed, or because
	 * the two changes were non-conflicting and mergeable.  These items
	 * will be written as staged entries in the main index.
	 */
	git_vector staged;

	/* Vector of git_merge_diff entries that represent the conflicts that
	 * have not been automerged.  These items will be written to high-stage
	 * entries in the main index.
	 */
	git_vector conflicts;

	/* Vector of git_merge_diff that have been automerged.  These items
	 * will be written to the REUC when the index is produced.
	 */
	git_vector resolved;
} git_merge_diff_list;

/**
 * Hold the list of git_merge_conflict
 */
struct git_merge_conflicts {
	git_merge_diff *diffs;
	size_t length;
};

int git_merge__bases_many(
	git_commit_list **out,
	git_revwalk *walk,
	git_commit_list_node *one,
	git_vector *twos);

/*
 * Three-way tree differencing
 */

git_merge_diff_list *git_merge_diff_list__alloc(git_repository *repo);

int git_merge_diff_list__find_differences(
	git_merge_diff_list *merge_diff_list,
	git_iterator *ancestor_iterator,
	git_iterator *ours_iter,
	git_iterator *theirs_iter);

int git_merge_diff_list__find_renames(git_repository *repo, git_merge_diff_list *merge_diff_list, const git_merge_options *opts);

void git_merge_diff_list__free(git_merge_diff_list *diff_list);

/* Merge metadata setup */

int git_merge__setup(
	git_repository *repo,
	const git_annotated_commit *our_head,
	const git_annotated_commit *heads[],
	size_t heads_len);

int git_merge__iterators(
        git_index **out,
        git_merge_conflicts **conflicts_out,
        git_repository *repo,
        git_iterator *ancestor_iter,
        git_iterator *our_iter,
        git_iterator *their_iter,
        const git_merge_options *given_opts);

int git_merge__check_result(git_repository *repo, git_index *index_new);

int git_merge__append_conflicts_to_merge_msg(git_repository *repo, git_index *index);

/* Merge files */

GIT_INLINE(const char *) git_merge_file__best_path(
	const char *ancestor,
	const char *ours,
	const char *theirs)
{
	if (!ancestor) {
		if (ours && theirs && strcmp(ours, theirs) == 0)
			return ours;

		return NULL;
	}

	if (ours && strcmp(ancestor, ours) == 0)
		return theirs;
	else if(theirs && strcmp(ancestor, theirs) == 0)
		return ours;

	return NULL;
}

GIT_INLINE(uint32_t) git_merge_file__best_mode(
	uint32_t ancestor, uint32_t ours, uint32_t theirs)
{
	/*
	 * If ancestor didn't exist and either ours or theirs is executable,
	 * assume executable.  Otherwise, if any mode changed from the ancestor,
	 * use that one.
	 */
	if (!ancestor) {
		if (ours == GIT_FILEMODE_BLOB_EXECUTABLE ||
			theirs == GIT_FILEMODE_BLOB_EXECUTABLE)
			return GIT_FILEMODE_BLOB_EXECUTABLE;

		return GIT_FILEMODE_BLOB;
	} else if (ours && theirs) {
		if (ancestor == ours)
			return theirs;

		return ours;
	}

	return 0;
}

#endif
