#!/usr/bin/perl

use strict;
use LWP::Simple;

my $base = 'http://www.pierceive.com/filtersetg/';

sub get_latest()
{
	my $changelog = "$base/changelog.txt";
	my $raw = get($changelog) or die("Could not download $changelog\n");
	my @doc = split /(\r|\n)/m, $raw;
	my $path = sprintf "$base/%s.txt", $doc[0];

	return get($path);
}

sub rewrite_regex
{
	if (m/^#/) { return $_; }
	if (m/^\[Adblock\]/) { return $_ = ""; }
	if (m/^\!Filterset/) { return $_ = ""; }
	if (m@^/(.*)/$@) { return $_ = $1; }
	return s/\./\\./g;
}

sub rewrite_file()
{
	my $doc = get_latest();
	my $first_line = 1;
	my $out = "";

	foreach (split /\r?\n/, $doc)
	{
		rewrite_regex();

		if ($_) {
			if ($first_line) {
				$first_line = 0;
			} else {
				$out .= "\n";
			}
			$out .= $_;
		}
	}

	return $out;
}

printf <<EOT, rewrite_file();
# Each line of this file is a Perl-compatible regular expression to match. If a
# requested URL matches any line in this file, the data at that URL will not be
# downloaded.
#
# For example, to block GIF images, use "\\.gif\$".
#
# This list is Copyright © its original author ("G"). It is relicensed under
# the GPL with permission.
#
# The original list, Filterset.G, can be found at
# $base/.
#
# This version has been modified from its original. The changes have not been
# approved by the original author; direct any problems to Bugzilla and not to
# the original author ("G").

%s
EOT
