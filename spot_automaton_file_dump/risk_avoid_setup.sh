success=true
echo $0
if [[ "$CONDA_PREFIX" == *"tpenv"* ]]; then
    echo "Found conda environment!"
    python formula2dfa.py -l risk_avoid_10x10 --complete --dfa-filename dfa_complete_liveness
else
    echo "Conda environment not found! (tpenv)"
fi