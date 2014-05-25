cp Reporte.tex tmp.tex
pdflatex -interaction=batchmode tmp.tex
pdflatex -interaction=batchmode tmp.tex
mv tmp.pdf Reporte.pdf
rm tmp.*
