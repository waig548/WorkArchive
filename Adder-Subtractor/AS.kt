import java.io.FileWriter
import kotlin.math.pow

object MPFA
{
    fun forward(m: Boolean, a: List<Boolean>, b: List<Boolean>): Pair<Boolean, List<Boolean>>
    {
        var i = 0
        var c = m
        val sl = mutableListOf<Boolean>()
        val tb = b.map { it xor c}
        while(i < 4)
        {
            val (s, tc) = FA.forward(a[i], tb[i], c)
            c = tc
            sl+=s
            i++
        }
        return c to sl
    }
}
object FA
{
    fun forward(a: Boolean, b: Boolean, c: Boolean): Pair<Boolean, Boolean>
    {
        return (a xor b xor c) to ((a and b) or (b and c) or (a and c))
    }
}

fun main()
{
    //println(FA.forward(true, false, true))
    //println(MPFA.forward(true, "0101".toBooleanArray(), "0101".toBooleanArray()))
    val csv = FileWriter("truthTable.csv")
    csv.appendLine("K, A3, A2, A1, A0, B3, B2, B1, B0, C, S3, S2, S1, S0")
    var i = 0
    while(i< (2.0).pow(9))
    {
        val inputStr = i.toString(2).reversed().let {it+("0".repeat(9-it.length))}.reversed()
        val rawInput = inputStr.toBooleanArray().reversed()
        val rawOutput = MPFA.forward(rawInput[0], rawInput.subList(1, 5).reversed(), rawInput.subList(5, 9).reversed())
        val output = rawOutput.first to rawOutput.second.reversed()
        csv.appendLine((rawInput to output).toString().removeBrackets().replaceBooleans())
        i++
    }
}
fun String.toBooleanArray() = this.reversed().map {it=='1'}

fun String.removeBrackets() = this.replace("[", "").replace("]", "").replace("(", "").replace(")", "")

fun String.replaceBooleans() = this.replace("false", "0").replace("true", "1")