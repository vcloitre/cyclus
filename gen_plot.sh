rm plot.png; rm prey; rm pred; rm cyclus.sqlite;
cyclus ./tests/input/lotka_volterra.xml -v LEV_INFO2 &&
cycpost cyclus.sqlite && 
metric -db=cyclus.sqlite deployseries Predator > pred &&
metric -db=cyclus.sqlite deployseries Prey > prey &&
./plot.p
