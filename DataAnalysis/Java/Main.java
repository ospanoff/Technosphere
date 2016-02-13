import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;
import java.lang.Math;
import  java.io.InputStream;

public class Main {

    static class Reader {
        public List<Integer> getData(InputStream inp)
        {
            List<Integer> list = new ArrayList<Integer>();
            Scanner in = new Scanner(inp);
            while (in.hasNextInt()) {
                list.add(in.nextInt());
            }
            return list;
        }
    }

    public static int getArea(List<Integer> list)
    {
        int currPtr = 0;
        int nextPtr = 1;
        while (nextPtr < list.size() && list.get(currPtr) <= list.get(nextPtr)) {
            currPtr++;
            nextPtr++;
        }
        int area = 0;
        while (true) {
            while (nextPtr + 1 < list.size() && list.get(nextPtr) >= list.get(nextPtr + 1)) {
                nextPtr++;
            }

            if (nextPtr + 1 >= list.size())
                break;

            int min = Math.min(list.get(currPtr), list.get(nextPtr + 1));
            for (int i = currPtr + 1; i < nextPtr + 1; i++) {
                int diff = min - list.get(i);
                if (diff > 0) {
                    area += diff;
                    list.set(i, min);
                }
            }
            if (list.get(currPtr) <= list.get(nextPtr + 1)) {
                currPtr = nextPtr + 1;
                nextPtr = currPtr + 1;
                while (nextPtr < list.size() && list.get(currPtr) <= list.get(nextPtr)) {
                    currPtr++;
                    nextPtr++;
                }
            } else {
                nextPtr++;
            }
        }
        return area;
    }

    public static void main(String[] args) {

        Reader reader = new Reader();
        List<Integer> list = reader.getData(System.in);

        System.out.print(getArea(list));
    }
}

