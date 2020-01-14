#include "LanguageBase.h"
#include <map>
#include <functional>
#include <cmath>
#include "unicode/uchar.h"


using std::map;
using std::multimap;
using std::greater;
using iknow::base::String;
using iknow::base::Char;
using iknow::base::SpaceString;

using namespace iknow::ali;

LanguageBase::~LanguageBase()
{
}

Char LanguageBase::NextClusterChar(const Char*& current_char, const Char* end, Char last_cluster_char) {
  //TRW: Copied and modified from LanguageBaseTextReader::getNextChar()
  Char nextChar = 0;
  // skip some characters
  while ( current_char != end &&
	  (
	   (*current_char >= L'0' && *current_char <= L'9') ||
	   *current_char == L'"' || *current_char == 223  ||
	   *current_char == 224 || *current_char == 147 ||
	   *current_char == 148 || (*current_char >= 8220 && *current_char <= 8224) ||
	   *current_char == L'(' || *current_char == L')'  ||
	   *current_char == L'{' || *current_char == L'}'  ||
	   *current_char == L'[' || *current_char == L']'  ||
	   *current_char == 253 || *current_char == 273
	   )
	  )
    ++current_char;
  if (current_char != end)
    {
      if (*current_char <= L' ') // everything before or equal then ascii char 32
	{
	  if ((last_cluster_char && last_cluster_char != L' ') || !last_cluster_char)
	    {
	      nextChar = L' ';
	    }
	}
      else if (*current_char == L'?' || *current_char == L'!' || *current_char == L';' || *current_char == L':' || *current_char == L'.' || *current_char == 205 || *current_char == 133)
	{
	  if ((last_cluster_char && last_cluster_char != L'.') || !last_cluster_char)
	    {
	      nextChar = L'.';
	    }
	}
      else
	{
	  if ((last_cluster_char && last_cluster_char != L' ') || !last_cluster_char)
	    //TODO: TRW, Use ICU lowercasing.
	    nextChar = static_cast<Char>(u_tolower(*current_char));
	  else
	    nextChar = *current_char;
	}
      //since we're not at the end, advance 
      ++current_char;
    }
  return nextChar;
}


double LanguageBase::GetSimilarity(const Char* c, size_t n) {
  size_t str_total_score = 0; //the count of ALI-processed clusters
  size_t match_score = 0; //the total score of lb1 clusters that match the string
  Char last_char = 0;
  const Char* pos = c;
  const Char* end = c + n;
  //This algorithm assumes a single "cluster size" equal to the max length (currently 
  //hard-wired to 4).
  //If this changes, the algorithm will need to be adjusted.
  const size_t cluster_size = 4;
  String cluster;
  cluster.reserve(cluster_size);
  //prepend a " "
  cluster += ' ';
  //track the location of the last alphabetic character we saw
  size_t last_alpha_char = 0;
  while (pos != end) {
    Char cur_char = NextClusterChar(pos, end, last_char);
    if (!cur_char) continue;
    cluster += cur_char;
    //If the character is alphanumeric, it will be around for cluster_size cycles
    if ((cur_char >= L'A') && (cur_char <= L'z')) {
      last_alpha_char = cluster_size;
    }
    //For non-ASCII chars, check ICU
    else if (cur_char > 127 && u_isalpha(cur_char)) {
      last_alpha_char = cluster_size;
    }
    if ((cluster.size() == cluster_size) && last_alpha_char) {
      str_total_score += 1;
      match_score += ScoreFor(cluster.data(), cluster.size());
    }
    //rotate the "buffer"
    if (cluster.size() >= cluster_size) {
      cluster.erase(0,1);
    }
    last_char = cur_char;
    if (last_alpha_char) --last_alpha_char;
  }
  return str_total_score?1000000.0*(double)match_score/(double)TotalScore()/(double)str_total_score:0;
}
