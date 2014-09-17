cp report.tex tmp.tex
pdflatex tmp.tex
pdflatex tmp.tex
mv tmp.pdf report.pdf
rm tmp.*
