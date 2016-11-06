#include <Python.h>
#include <numpy/numpyconfig.h>
#include <numpy/arrayobject.h>
#include <stdio.h>

static PyObject *TestError;

#include <stdio.h>

static float* bandpass(float *values, long numLines)
{
	float *res = malloc(sizeof(float) * numLines);
	float f;
	float fb;
	float t;
	float f4;
	float in1;
	float in2;
	float in3;
	float in4;
	float out1;
	float out2;
	float out3;
	float out4;
	float value;
	long num;

	f = 0.9 * 1.16;
	fb = 0.5 * (1.0 - 0.15 * f * f);
	t = 1 - f;
	f4 = f*f*f*f;
	for (num=0; num < numLines; num++)
	{
		in1 = 0;
		in2 = 0;
		in3 = 0;
		in4 = 0;
		out1 = 0;
		out2 = 0;
		out3 = 0;
		out4 = 0;
		if (num > 0)
		{
			value = values[num - 1];
			value -= out4 * fb;
			value *= 0.35013 * f4;
			out1 = value + 0.3 * in1 + t * out1;
			in1  = value;
			out2 = out1 + 0.3 * in2 + t * out2;
			in2  = out1;
			out3 = out2 + 0.3 * in3 + t * out3;
			in3  = out2;
			out4 = out3 + 0.3 * in4 + t * out4;
			in4  = out3;
		}
		value = values[num];
		value -= out4 * fb;
		value *= 0.35013 * f4;
		out1 = value + 0.3 * in1 + t * out1;
		in1  = value;
		out2 = out1 + 0.3 * in2 + t * out2;
		in2  = out1;
		out3 = out2 + 0.3 * in3 + t * out3;
		in3  = out2;
		out4 = out3 + 0.3 * in4 + t * out4;
		in4  = out3;
		res[num] = (3.0 * (out3 - out4));
	}
	return res;
}

static PyObject* getVolumeInfo(float *envelope, long numLines)
{
	float minE = 1;
	float threshold = 1.1;
	float lowEnd = 0.09;
	float highEnd = 0.6;
	PyObject *bad = PyList_New(4);
	float current = 0.0;
	float currcount = 0;
	float prev = 0;
	float e;
	PyObject *list;
	PyObject *temp;
	long i;
	float maxtime;
	float belowThreshold;
	int count;
	float total;

	PyList_SetItem(bad, 0, PyList_New(0));
	PyList_SetItem(bad, 1, PyList_New(0));
	for (i=0; i < numLines; i++){
		e = envelope[i];
		if (minE > e && e > 0.05)
			minE = e;
	}
	threshold *= minE;
	maxtime = 44100 * .2;
	belowThreshold = 0;
	count = 0;
	total = 0.0;
	for (i=0; i < numLines; i++)
	{
		e = envelope[i];
		total += e;
		count += 1;
		if (e < threshold)
			belowThreshold++;
		else
		{
			current += e;
			currcount += 1;
			belowThreshold = 0;
		}
		if (belowThreshold == maxtime)
		{
			current /= currcount;
			if (current < lowEnd)
			{
				temp = PyList_New(2);
				PyList_SetItem(temp, 0, PyFloat_FromDouble(prev));
				PyList_SetItem(temp, 1, PyFloat_FromDouble(count));
				PyList_Append(PyList_GetItem(bad, 0), temp);
			}
			else if (current > highEnd)
			{
				temp = PyList_New(2);
				PyList_SetItem(temp, 0, PyFloat_FromDouble(prev));
				PyList_SetItem(temp, 1, PyFloat_FromDouble(count));
				PyList_Append(PyList_GetItem(bad, 1), temp);
			}
			prev = count;
			current = 0.0;
			currcount = 0;
		}
	}
	PyList_SetItem(bad, 2, PyFloat_FromDouble(total / count));
	return bad;
}

float *getEnvelope(void *in, long numLines, char type)
{
	float a = 0.99;
	float b = 1.0 - a;
	double prev = 0;
	double current = 0;
	float z;
	PyObject *floatObj;
	float *ret;
	long i;
	PyObject *res;

	ret = malloc(sizeof(float) * numLines);
	if (numLines < 0)
		return NULL;
	for (i=0; i < numLines; i++){
		if (!type)
		{
			floatObj = PyList_GetItem(in, i);
			current = PyFloat_AsDouble(floatObj);
		}
		else
			current = ((float *)in)[i];
		z = prev;
		z = fabs(current * b) + fabs(z) * a;
		ret[i] = (float)z;
		prev = current;
	}
	return ret;
}

static PyObject* doAll(PyObject* self, PyObject* args)
{
	PyObject *list;
	float *ret;
	float *bpEnvelope;
	Py_ssize_t numLines;
	PyObject *res;

	if (! PyArg_ParseTuple( args, "O!", &PyList_Type, &list, 0, 0))
		return NULL;
	numLines = PyList_Size(list);
	ret = getEnvelope(list, numLines, 0);
	float *bp = bandpass(ret, numLines);
	bpEnvelope = getEnvelope(bp, numLines, 1);
	res = getVolumeInfo(ret, numLines);
	// long i;
	// for (i=0; i < numLines; i++)
	// 	if (bpEnvelope[i] > 0.001)
	// 		printf("%d, %f\n", i, bpEnvelope[i]);
	// printf("\nTest: %f\n", bpEnvelope[374335]);
	return res;
}

static PyMethodDef TestMethods[] = {
    {"doAll",  doAll, METH_VARARGS,
     "Does something cool."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC
initbp(void)
{
    PyObject *m;

	import_array();
    m = Py_InitModule("bp", TestMethods);
    if (m == NULL)
        return;

    TestError = PyErr_NewException("bp.error", NULL, NULL);
    Py_INCREF(TestError);
    PyModule_AddObject(m, "error", TestError);
}

int main(int ac, char **av)
{
	ac = 0;
	av = 0;
	
	return (0);
}