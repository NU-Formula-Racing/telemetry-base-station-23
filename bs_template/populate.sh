if [[ -d ../$1 ]]; then
	echo "Transferring template into $1"
	echo "..."
	cp platformio.ini ../$1/platformio.ini
	for header in *.h; do
		cp $header ../$1/include/$header
	done
	for program in *.cpp; do
		cp $program ../$1/src/$program
	done
	echo "Okie dokie lokie!"
fi
