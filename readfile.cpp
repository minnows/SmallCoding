#include <iostream>
#include <fstream>
#include <assert.h>
#include <sstream>

using namespace std;


int FindDataFlag(const char *line)
{
    int dataIndex = 0;

    while ( (':' != line[dataIndex] ) && ( 200 > dataIndex ) )
    {
        dataIndex++;
    }

    if ( ':' == line[dataIndex] )
    {
        return dataIndex;
    }
    else
    {
        return 0;
    }
}

void GetData(char* source, int* data)
{
    char strData[4];
    char *strPos = strData;
    int dataIndex = 0;
    cout << "extract data from " << source << endl;
    while (*source)
    {
        if ( ',' == *source )
        {
            data[dataIndex] = stoi(strData);
            memset(strData, ' ', sizeof(strData));
            strPos = strData;
            dataIndex++;
        }
        else if (' ' == *source)
        {

        }
        else
        {
            *strPos = *source;
            strPos++;
        }
        source++;
    }

    if (strPos != strData)
    {
        data[dataIndex] = stoi(strData);       
    }

}

int main(int argc, char const *argv[])
{
    cout << "start reading data from file ..." << endl << endl;
    ifstream dataFile("data.dat");
    assert(dataFile);
    cout << "file open success ..." << endl << endl;
    char text[200];
    int line = 0;
    int data[4][200];
    while(dataFile.getline(text, 200))
    {
        cout << "this is line " << line+1 << " of data.dat:" << endl;
        cout << text << endl;
        int flagPos = FindDataFlag(text);
        cout << "position of ':' is " << flagPos << endl;
        GetData(&text[flagPos+2], data[line]);
        cout << "data in this line: " << data[line][0] << ", "
                                      << data[line][1] << ", "
                                      << data[line][2] << ", "
                                      << data[line][3] << ", "
                                      << endl << endl;
        line++;
    }

    if ( 0 == line )
    {
        cout << "data.dat is empty" << endl;
    }

    return 0;
}




