#include "Statistics.h"
#include "Helper.h"

Statistics::Statistics()
{
}

Statistics::Statistics(Statistics &copyMe)
{
}

Statistics::~Statistics()
{
}

// Adds a new relation, specifying its name and the number of tuples
void Statistics::AddRel(char *relName, int numTuples)
{
    relation[string(relName)].numAttr = numTuples;
    estimate[string(relName)] = numTuples;
}

// Adds a new attribute for a relation, specifying its name and the number of distincts
void Statistics::AddAtt(char *relName, char *attName, int numDistincts)
{
    relation[string(relName)].AddAtt(string(attName), numDistincts);
}

// Produces a copy of the relation (including all of its attributes and all of its statistics) and stores it under the new name.
void Statistics::CopyRel(char *oldName, char *newName)
{
    string newRelation(newName);
    string oldRelation(oldName);

    relation[newRelation].numAttr = relation[oldRelation].numAttr;
    for (auto iter:relation[oldRelation].attr)
    {
        relation[newRelation].attr[newRelation + "." + iter.first] = iter.second;
    }
    estimate[newRelation] = relation[newRelation].numAttr;
}

// Read data from file and stores in object
void Statistics::Read(char *fromWhere)
{
    ifstream in(fromWhere);
    in >> *this;
    in.close();
}

// Writes object data to file
void Statistics::Write(char *fromWhere)
{
    ofstream out(fromWhere);
    out << *this;
    out.close();
}

// The  Apply operation uses the statistics stored by the Statistics class to simulate a join of all of the
// relations listed in the relNames parameter.
void Statistics::Apply(struct AndList *parseTree, char *relNames[], int numToJoin)
{
    if (!checkRel(relNames, numToJoin))
        return;
    struct AndList *pA = parseTree;
    double fact = 1.0, orFact = 0, orFactDe = 0, orFactPartial = 0;
    while (pA != NULL)
    {
        struct OrList *pOr = pA->left;
        orFact = 0;
        orFactDe = 1;
        unordered_map<string, int> diff;
        string previous;
        while (pOr != NULL)
        {
            struct ComparisonOp *pComp = pOr->left;
            double tLeft = 1.0, tRight = 1.0;
            if (!checkAtt(pComp->left, tLeft, relNames, numToJoin) ||
                !checkAtt(pComp->right, tRight, relNames, numToJoin))
            {
                return;
            }
            string oper1(pComp->left->value);

            if (pComp->code == 1 || pComp->code == 2)
            {
                orFact += 1.0 / 3;
                orFactDe *= 1.0 / 3;
                for (auto &iter:relation)
                    if (iter.second.attr.count(oper1) != 0)
                        iter.second.attr[oper1] /= 3;

                pOr = pOr->rightOr;
                continue;
            }
            if (tRight == 1.0 || tLeft > tRight || tLeft < tRight)
            {
                if (diff.count(oper1))
                {
                    diff[oper1]++;
                }
                else
                {
                    diff[oper1] = min(tLeft, tRight);
                }

            }
            if (oper1.compare(previous) != 0 && orFactPartial != 0)
            {
                orFact += orFactPartial;
                orFactDe *= orFactPartial;
                orFactPartial = 0;

            }
            orFactPartial += 1.0 / max(tLeft, tRight);
            previous = oper1;

            pOr = pOr->rightOr;
        }

        if (orFactPartial)
        {
            orFact += orFactPartial;
            orFactDe *= orFactPartial;
            orFactPartial = 0;
        }

        if (orFact != orFactDe)
        {
            fact *= (orFact - orFactDe);
        }
        else
        {
            fact *= orFact;
        }
        for (auto &ch:diff)
            for (auto &rel:relation)
                if (rel.second.attr.count(ch.first))
                {
                    rel.second.attr[ch.first] = ch.second;
                }
        pA = pA->rightAnd;
    }

    long double maxTup = 1;
    bool reltable[numToJoin];
    for (int i = 0; i < numToJoin; i++)
    {
        reltable[i] = true;
    }

    for (int i = 0; i < numToJoin; i++)
    {
        if (!reltable[i])
        {
            continue;
        }
        string relname(relNames[i]);
        for (auto iter = estimate.begin(); iter != estimate.end(); iter++)
        {
            auto check = split(iter->first);
            if (check.count(relNames[i]) != 0)
            {
                reltable[i] = false;
                maxTup *= iter->second;
                for (int j = i + 1; j < numToJoin; j++)
                {
                    if (check.count(relNames[j]) != 0)
                        reltable[j] = false;
                }
                break;
            }
        }
    }

    double result = fact * maxTup;

    string newSet;
    for (int i = 0; i < numToJoin; i++)
    {
        string relname(relNames[i]);
        newSet += relname + "#";
        for (auto iter = estimate.begin(); iter != estimate.end(); iter++)
        {
            auto check = split(iter->first);
            if (check.count(relname) != 0)
            {
                estimate.erase(iter);
                break;
            }
        }
    }
    estimate.insert({newSet, result});
}

// It  computes  the  number  of  tuples  that  would  result from a join over the relations in relNames,
// and returns this to the caller
double Statistics::Estimate(struct AndList *parseTree, char **relNames, int numToJoin)
{
    if (!checkRel(relNames, numToJoin))
    {
        return 0;
    }
    struct AndList *pA = parseTree;
    double fact = 1.0;
    double orFact = 0;
    double orFactDe = 0;
    double orFactPartial = 0;
    while (pA != NULL)
    {
        struct OrList *pOr = pA->left;
        orFact = 0;
        orFactDe = 1;
        string previous;
        while (pOr != NULL)
        {
            struct ComparisonOp *pComp = pOr->left;
            double tLeft = 1.0, tRight = 1.0;
            if (!checkAtt(pComp->left, tLeft, relNames, numToJoin) ||
                !checkAtt(pComp->right, tRight, relNames, numToJoin))
                return 0;
            if (pComp->code == 1 || pComp->code == 2)
            {
                orFact += 1.0 / 3;
                orFactDe *= 1.0 / 3;
                pOr = pOr->rightOr;
                continue;
            }

            string oper1(pComp->left->value);
            if (oper1.compare(previous) != 0 && orFactPartial != 0)
            {
                orFact += orFactPartial;
                orFactDe *= orFactPartial;
                orFactPartial = 0;
            }
            orFactPartial += 1.0 / max(tLeft, tRight);
            previous = oper1;
            pOr = pOr->rightOr;
        }
        if (orFactPartial != 0)
        {
            orFact += orFactPartial;
            orFactDe *= orFactPartial;
            orFactPartial = 0;
        }
        if (orFact != orFactDe)
        {
            fact *= (orFact - orFactDe);
        }
        else
        {
            fact *= orFact;
        }
        pA = pA->rightAnd;
    }

    long double maxTup = 1;
    vector<bool> reltable(numToJoin, true);
    for (int i = 0; i < numToJoin; i++)
    {
        if (!reltable[i])
        {
            continue;
        }

        string relname(relNames[i]);

        for (auto iter = estimate.begin(); iter != estimate.end(); iter++)
        {
            auto check = split(iter->first);
            if (check.count(relNames[i]))
            {
                reltable[i] = false;
                maxTup *= iter->second;

                for (int j = 0; j < numToJoin; j++)
                {
                    if (check.count(relNames[j]))
                    {
                        reltable[j] = false;
                    }

                }
                break;
            }
        }
    }
    return fact * maxTup;
}

// Inserts the number of attributes of the relation and the relation size in the output stream
std::ostream &operator<<(std::ostream &os, const Rel &relation)
{
    os << relation.numAttr << "\n" << relation.attr.size() << "\n";
    for (auto i:relation.attr)
    {
        os << i.first << "\n" << i.second << "\n";
    }
    return os;
}

// Reads the number of attributes in the relation from the input stream
std::istream &operator>>(std::istream &is, Rel &Rel)
{
    int size;
    is >> Rel.numAttr >> size;
    for (int i = 0; i < size; i++)
    {
        string temp;
        is >> temp;
        is >> Rel.attr[temp];
    }
    return is;
}

// Adds the size of the relation present inside Statistics as well as the attribute names
// and values to the output stream followed by the estimate
std::ostream &operator<<(std::ostream &os, const Statistics &stat)
{
    os << stat.relation.size() << "\n";
    for (auto iter:stat.relation)
        os << iter.first << "\n" << iter.second;
    os << save_two_values<unordered_map < string, double>>
    (stat.estimate) << "\n";
    return os;
}

// Reads all the relations present in the file and stores them in the statistics object
std::istream &operator>>(std::istream &is, Statistics &stat)
{
    int size;
    is >> size;
    for (int i = 0; i < size; i++)
    {
        string temp;
        struct Rel tempRel;
        is >> temp >> tempRel;
        stat.relation[temp] = tempRel;
        //stat.estimate[temp]=tempRel.numAttr;
    }
    is >> size;
    for (int i = 0; i < size; i++)
    {
        string temp;
        double dbl;
        is >> temp >> dbl;
        stat.estimate[temp] = dbl;
    }
    return is;
}