success=true
echo $0
if [[ "$CONDA_PREFIX" == *"tpenv"* ]]; then
    echo "Found conda environment!"
    python formula2dfa.py --complete --dfa_filename dfa_complete_liveness
else
    echo "Conda environment not found! (tpenv)"
fi