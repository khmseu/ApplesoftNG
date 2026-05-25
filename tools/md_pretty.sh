#!/usr/bin/env bash
set -euo pipefail

while IFS= read -r -d '' file; do
	[[ ${file} == *.md ]] || continue
	[[ ${file} == legacy/* ]] && continue

	printf '%s' "${file}"

	tmp_file="$(mktemp)"
	npx markdown-table-prettify <"${file}" >"${tmp_file}"
	trunk fmt "${tmp_file}" >/dev/null

	if ! cmp -s "${tmp_file}" "${file}"; then
		mv "${tmp_file}" "${file}"
	else
		rm -f "${tmp_file}"
	fi

	printf "\n"
done < <(
	# shellcheck disable=SC2312
	git ls-files -z
)
