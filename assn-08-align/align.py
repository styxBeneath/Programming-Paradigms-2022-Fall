#!/usr/bin/env python

import random # for seed, random
import sys    # for stdout



################################### TEST PART ##################################
################################################################################

# Tests align strands and scores
# Parameters types:
#    score          =  int   example: -6
#    plusScores     = string example: "  1   1  1"
#    minusScores    = string example: "22 111 11 "
#    strandAligned1 = string example: "  CAAGTCGC"
#    strandAligned2 = string example: "ATCCCATTAC"
#
#   Note: all strings must have same length
def test(score, plusScores, minusScores, strandAligned1, strandAligned2):
    print("\n>>>>>>START TEST<<<<<<")

    if testStrands(score, plusScores, minusScores, strandAligned1, strandAligned2):
        sys.stdout.write(">>>>>>>Test SUCCESS:")
        sys.stdout.write("\n\t\t" + "Score: "+str(score))
        sys.stdout.write("\n\t\t+ " + plusScores)
        sys.stdout.write("\n\t\t  " + strandAligned1)
        sys.stdout.write("\n\t\t  " + strandAligned2)
        sys.stdout.write("\n\t\t- " + minusScores)
        sys.stdout.write("\n\n")
    else:
        sys.stdout.write("\t>>>>!!!Test FAILED\n\n")


# converts character score to int
def testScoreToInt(score):
    if score == ' ':
        return 0
    return int(score)


# computes sum of scores
def testSumScore(scores):
    result = 0
    for ch in scores:
        result += testScoreToInt(ch)
    return result


# test each characters and scores
def testValidateEach(ch1, ch2, plusScore, minusScore):
    if ch1 == ' ' or ch2 == ' ':
        return plusScore == 0 and minusScore == 2
    if ch1 == ch2:
        return plusScore == 1 and minusScore == 0
    return plusScore == 0 and minusScore == 1


# test and validates strands
def testStrands(score, plusScores, minusScores, strandAligned1, strandAligned2):
    if len(plusScores) != len(minusScores) or len(minusScores) != len(strandAligned1) or len(strandAligned1) != len(
            strandAligned2):
        sys.stdout.write("Length mismatch! \n")
        return False

    if len(plusScores) == 0:
        sys.stdout.write("Length is Zero! \n")
        return False

    if testSumScore(plusScores) - testSumScore(minusScores) != score:
        sys.stdout.write("Score mismatch to score strings! TEST FAILED!\n")
        return False
    for i in range(len(plusScores)):
        if not testValidateEach(strandAligned1[i], strandAligned2[i], testScoreToInt(plusScores[i]),
                                testScoreToInt(minusScores[i])):
            sys.stdout.write("Invalid scores for position " + str(i) + ":\n")
            sys.stdout.write("\t char1: " + strandAligned1[i] + " char2: " +
                             strandAligned2[i] + " +" + str(testScoreToInt(plusScores[i])) + " -" +
                             str(testScoreToInt(minusScores[i])) + "\n")
            return False

    return True

######################## END OF TEST PART ######################################
################################################################################


# Computes the score of the optimal alignment of two DNA strands.
def findOptimalAlignment(strand1, strand2):
	dict = {}
	return findOptimalAlignmentRec(strand1, strand2, dict)

def findOptimalAlignmentRec(strand1, strand2, dict):
	if len(strand1) == 0:
		return [len(strand2) * -2, len(strand2) * ' ', strand2]
	
	if len(strand2) == 0:
		return [len(strand1) * -2, strand1, len(strand1) * ' ']

	if dict.has_key(strand1 + '-' + strand2):
		return dict[strand1 + '-' + strand2]

	both = findOptimalAlignmentRec(strand1[1:], strand2[1:], dict)
	if strand1[0] == strand2[0]:
		dict[strand1 + '-' + strand2] = both[0] + 1, strand1[0] + both[1], strand2[0] + both[2]
		return [both[0] + 1, strand1[0] + both[1], strand2[0] + both[2]]
	
	res = [both[0] - 1, strand1[0] + both[1], strand2[0] + both[2]]
	withoutFirst = findOptimalAlignmentRec(strand1, strand2[1:], dict)
	if withoutFirst[0] - 2 > res[0]:
		res = [withoutFirst[0] - 2, ' ' + withoutFirst[1], strand2[0] + withoutFirst[2]]

	without_sec = findOptimalAlignmentRec(strand1[1:], strand2, dict)
	if without_sec[0] - 2 > res[0]:
		res = [without_sec[0] - 2, strand1[0] + without_sec[1], ' ' + without_sec[2]]

	dict[strand1 + '-' + strand2] = res
	return res

# Utility function that generates a random DNA string of
# a random length drawn from the range [minlength, maxlength]
def generateRandomDNAStrand(minlength, maxlength):
	assert minlength > 0, \
	       "Minimum length passed to generateRandomDNAStrand" \
	       "must be a positive number" # these \'s allow mult-line statements
	assert maxlength >= minlength, \
	       "Maximum length passed to generateRandomDNAStrand must be at " \
	       "as large as the specified minimum length"
	strand = ""
	length = random.choice(xrange(minlength, maxlength + 1))
	bases = ['A', 'T', 'G', 'C']
	for i in xrange(0, length):
		strand += random.choice(bases)
	return strand

# Method that just prints out the supplied alignment score.
# This is more of a placeholder for what will ultimately
# print out not only the score but the alignment as well.
def printAlignment(score, out = sys.stdout):
	out.write("Optimal alignment score is " + str(score) + "\n")


def computeScores(strand1, strand2):
	posAndNeg = ['', '']
	for x in xrange(len(strand1)):
		if strand1[x] == strand2[x]:
			posAndNeg[0] += '1'
			posAndNeg[1] += ' '
		elif strand1[x] == ' ' or strand2[x] == ' ':
			posAndNeg[0] += ' '
			posAndNeg[1] += '2'
		else:
			posAndNeg[0] += ' '
			posAndNeg[1] += '1'
	
	return posAndNeg

# Unit test main in place to do little more than
# exercise the above algorithm.  As written, it
# generates two fairly short DNA strands and
# determines the optimal alignment score.
#
# As you change the implementation of findOptimalAlignment
# to use memoization, you should change the 8s to 40s and
# the 10s to 60s and still see everything execute very
# quickly.
def main():
        test(-4,
             "  11 1 1 11 ",
             "12  2 2 1  2",
             "G ATCG GCAT ",
             "CAAT GTGAATC")
	while (True):
		sys.stdout.write("Generate random DNA strands? ")
		answer = sys.stdin.readline()
		if answer == "no\n": break
		strand1 = generateRandomDNAStrand(40, 60)
		strand2 = generateRandomDNAStrand(40, 60)
		sys.stdout.write("Aligning these two strands: " + strand1 + "\n")
		sys.stdout.write("                            " + strand2 + "\n")
		alignment = findOptimalAlignment(strand1, strand2)
		printAlignment(alignment[0])
        posAndNeg = computeScores(alignment[1], alignment[2])
        test(alignment[0], posAndNeg[0], posAndNeg[1], alignment[1], alignment[2])

if __name__ == "__main__":
  main()
