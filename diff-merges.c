#include "diff-merges.h"

#include "revision.h"

static void suppress(struct rev_info *revs)
{
	revs->separate_merges = 0;
	revs->first_parent_merges = 0;
	revs->combine_merges = 0;
	revs->dense_combined_merges = 0;
	revs->combined_all_paths = 0;
	revs->combined_imply_patch = 0;
	revs->merges_need_diff = 0;
}

static void set_separate(struct rev_info *revs)
{
	suppress(revs);
	revs->separate_merges = 1;
}

static void set_first_parent(struct rev_info *revs)
{
	set_separate(revs);
	revs->first_parent_merges = 1;
}

static void set_m(struct rev_info *revs)
{
	/*
	 * To "diff-index", "-m" means "match missing", and to the "log"
	 * family of commands, it means "show full diff for merges". Set
	 * both fields appropriately.
	 */
	set_separate(revs);
	revs->match_missing = 1;
}

static void set_combined(struct rev_info *revs)
{
	suppress(revs);
	revs->combine_merges = 1;
	revs->dense_combined_merges = 0;
}

static void set_dense_combined(struct rev_info *revs)
{
	suppress(revs);
	revs->combine_merges = 1;
	revs->dense_combined_merges = 1;
}

static void set_diff_merges(struct rev_info *revs, const char *optarg)
{
	if (!strcmp(optarg, "off") || !strcmp(optarg, "none")) {
		suppress(revs);
		return;
	}
	if (0) ;
	else if (!strcmp(optarg, "first") || !strcmp(optarg, "first-parent") ||
		 !strcmp(optarg, "1"))
		set_first_parent(revs);
	else if (!strcmp(optarg, "sep")   || !strcmp(optarg, "separate") ||
		 !strcmp(optarg, "m"))
		set_separate(revs);
	else if (!strcmp(optarg, "comb")  || !strcmp(optarg, "combined") ||
		 !strcmp(optarg, "c"))
		set_combined(revs);
	else if (!strcmp(optarg, "dense") || !strcmp(optarg, "dense-combined") ||
		 !strcmp(optarg, "cc"))
		set_dense_combined(revs);
	else
		die(_("unknown value for --diff-merges: %s"), optarg);
	revs->merges_need_diff = 1;
}

/*
 * Public functions. They are in the order they are called.
 */

int diff_merges_parse_opts(struct rev_info *revs, const char **argv)
{
	int argcount = 1;
	const char *optarg;
	const char *arg = argv[0];

	if (!strcmp(arg, "-m"))
		set_m(revs);
	else if (!strcmp(arg, "-c")) {
		set_combined(revs);
		revs->combined_imply_patch = 1;
	}
	else if (!strcmp(arg, "--cc")) {
		set_dense_combined(revs);
		revs->combined_imply_patch = 1;
	}
	else if (!strcmp(arg, "--no-diff-merges"))
		suppress(revs);
	else if (!strcmp(arg, "--combined-all-paths"))
		revs->combined_all_paths = 1;
	else if ((argcount = parse_long_opt("diff-merges", argv, &optarg)))
		set_diff_merges(revs, optarg);
	else
		return 0;

	revs->explicit_diff_merges = 1;
	return argcount;
}

void diff_merges_suppress(struct rev_info *revs)
{
	suppress(revs);
}

void diff_merges_default_to_first_parent(struct rev_info *revs)
{
	if (!revs->explicit_diff_merges)
		revs->separate_merges = 1;
	if (revs->separate_merges)
		revs->first_parent_merges = 1;
}

void diff_merges_default_to_dense_combined(struct rev_info *revs)
{
	if (!revs->explicit_diff_merges)
		set_dense_combined(revs);
}

void diff_merges_set_dense_combined_if_unset(struct rev_info *revs)
{
	if (!revs->combine_merges)
		set_dense_combined(revs);
}

void diff_merges_setup_revs(struct rev_info *revs)
{
	if (revs->combine_merges == 0)
		revs->dense_combined_merges = 0;
	if (revs->separate_merges == 0)
		revs->first_parent_merges = 0;
	if (revs->combined_all_paths && !revs->combine_merges)
		die("--combined-all-paths makes no sense without -c or --cc");

	if (revs->combined_imply_patch)
		revs->diff = 1;

	if (revs->combined_imply_patch || revs->merges_need_diff) {
		if (!revs->diffopt.output_format)
			revs->diffopt.output_format = DIFF_FORMAT_PATCH;
	}
}
