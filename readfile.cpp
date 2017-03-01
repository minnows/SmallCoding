#include <iostream>
#include <fstream>
#include <assert.h>
#include <sstream>
#include <cstring>
#include <string>

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
    std::cout << "extract data from " << source << std::endl;
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
    std::cout << "start reading data from file ..." << std::endl << std::endl;
    ifstream dataFile("data.dat");
    assert(dataFile);
    std::cout << "file open success ..." << std::endl << std::endl;
    char text[200];
    int line = 0;
    int data[4][200];
    while(dataFile.getline(text, 200))
    {
        std::cout << "this is line " << line+1 << " of data.dat:" << std::endl;
        std::cout << text << std::endl;
        int flagPos = FindDataFlag(text);
        std::cout << "position of ':' is " << flagPos << std::endl;
        GetData(&text[flagPos+2], data[line]);
        std::cout << "data in this line: " << data[line][0] << ", "
                                      << data[line][1] << ", "
                                      << data[line][2] << ", "
                                      << data[line][3] << ", "
                                      << std::endl << std::endl;
        line++;
    }

    if ( 0 == line )
    {
        std::cout << "data.dat is empty" << std::endl;
    }

    return 0;
}




