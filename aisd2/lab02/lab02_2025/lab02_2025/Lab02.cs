using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ASD
{
    public class Lab02 : MarshalByRefObject
    {
        /// <summary>
        /// Etap 1 - Wyznaczenie ścieżki (seam) o minimalnym sumarycznym score.
        /// Ścieżka przebiega od górnego do dolnego wiersza obrazu.
        /// </summary>
        /// <param name="S">macierz score o wymiarach H x W, gdzie S[i, j] reprezentuje "ważność" piksela w wierszu i i kolumnie j</param>
        /// <returns>
        /// (int cost, (int, int)[] seam) - 
        /// cost: minimalny łączny koszt ścieżki (suma wartości pikseli);
        /// seam: tablica pozycji pikseli (włącznie z pikselem z pierwszego i ostatniego wiersza) tworzących ścieżkę.
        /// </returns>
        public (int cost, (int i, int j)[] seam) Stage1(int[,] S)
        {
            int H = S.GetLength(0);
            int W = S.GetLength(1);

            return (int.MaxValue, null);
        }

        /// <summary>
        /// Etap 2 - Wyznaczenie ścieżki (seam) o minimalnym sumarycznym score z uwzględnieniem kary za zmianę kierunku.
        /// Przy każdym przejściu, gdy kierunek ruchu różni się od poprzedniego, do łącznego kosztu dodawana jest kara K.
        /// Pierwszy krok (z pierwszego wiersza) nie podlega karze.
        /// </summary>
        /// <param name="S">macierz score o wymiarach H x W</param>
        /// <param name="K">kara za zmianę kierunku (K >= 1)</param>
        /// <returns>
        /// (int cost, (int, int)[] seam) - 
        /// cost: minimalny łączny koszt ścieżki (suma wartości pikseli oraz naliczonych kar);
        /// seam: tablica pozycji pikseli tworzących ścieżkę.
        /// </returns>
        public (int cost, (int i, int j)[] seam) Stage2(int[,] S, int K)
        {
            int H = S.GetLength(0);
            int W = S.GetLength(1);

            return (int.MaxValue, null);
        }
    }
}
