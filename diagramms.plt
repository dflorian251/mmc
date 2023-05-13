# Set the output to a pdf file
set term pdfcairo
# The file we'll write to
set output 'n_system.pdf'
set xlabel "ρ"
set ylabel "number"
# The graphic title
set title 'DIKTYA'
set datafile separator ","
set grid
#plot the graphic
plot "diktya.csv" using 4:12 with lines title "N system Exp", \
    "diktya.csv" using 4:14 with lines title "N system Theory"
