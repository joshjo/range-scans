// discrete_distribution
#include <iostream>
#include <random>

#include "src/query.h"
#include "src/uitree.h"

#include "zipf.h"

typedef long long T;
typedef Interval<T> Tinterval;
typedef Query<T> Tquery;

int main()
{
  vector<Tquery *> queries;

  queries.push_back(new Tquery(Tinterval(750, 830)));
  queries.push_back(new Tquery(Tinterval(830, 860)));
  queries.push_back(new Tquery(Tinterval(520, 600)));
  queries.push_back(new Tquery(Tinterval(540, 550)));
  queries.push_back(new Tquery(Tinterval(530, 620)));
  queries.push_back(new Tquery(Tinterval(310, 360)));
  queries.push_back(new Tquery(Tinterval(300, 330)));
  queries.push_back(new Tquery(Tinterval(50, 120)));
  queries.push_back(new Tquery(Tinterval(90, 140)));
  queries.push_back(new Tquery(Tinterval(970, 980)));
  queries.push_back(new Tquery(Tinterval(950, 970)));
  queries.push_back(new Tquery(Tinterval(630, 1000)));

  QMapLazy <Traits <T>> * qMap = new QMapLazy <Traits <T>>();
  UITree <Traits <T> > * uitree = new UITree <Traits <T> >(1000, qMap);

  for (size_t i = 0; i < queries.size(); i++) {
    uitree->insert(queries[i]);
  }

  qMap->print();

  return 0;
}
