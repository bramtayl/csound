cmake --build build &> /tmp/iwyu.out
python3 ~/fix_includes.py --comments --nosafe_headers < /tmp/iwyu.out
