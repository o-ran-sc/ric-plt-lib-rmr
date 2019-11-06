
# format the changes file at the top level into xfm input
cat <<endKat
.im setup.im
&h1(RMR Release Notes)
The following is a list of release highlights for the core RMR library.
These are extracted directly from the CHANGES file at the repo root; 
please refer to that file for a completely up to date listing of
API changes.  
&space

endKat

sed 's/^/!/' ../../../CH*|awk ' 

	print_raw && /^!$/ { 
		printf( "&space\n\n" ); 
		next 
	} 

	{ gsub ( "!", "", $1 ) }

	$1 + 0 >= 2019 {
		print_raw = 1
		printf( "&h2(%s)\n", $0 )
		next 
	}

	print_raw { print } 
	'
